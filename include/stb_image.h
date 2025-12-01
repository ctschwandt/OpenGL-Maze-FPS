#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

const char *stbi_failure_reason(void);
unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
unsigned char *stbi_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *retval_from_stbi_load);
void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <png.h>
#include <jpeglib.h>

static int stbi__flip_vertically = 0;
static const char *stbi__last_failure = NULL;

static void stbi__set_failure(const char *reason)
{
    stbi__last_failure = reason;
}

const char *stbi_failure_reason(void)
{
    return stbi__last_failure;
}

void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip)
{
    stbi__flip_vertically = flag_true_if_should_flip;
}

void stbi_image_free(void *retval_from_stbi_load)
{
    free(retval_from_stbi_load);
}

// ----------------------- PNG loader helpers -----------------------
typedef struct
{
    const unsigned char *buffer;
    size_t size;
    size_t offset;
} stbi__png_stream;

static void stbi__png_read(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
{
    stbi__png_stream *stream = (stbi__png_stream *)png_get_io_ptr(png_ptr);
    if (stream->offset + byteCountToRead > stream->size)
        png_error(png_ptr, "Read past end of buffer");

    memcpy(outBytes, stream->buffer + stream->offset, byteCountToRead);
    stream->offset += byteCountToRead;
}

static unsigned char *stbi__png_load_from_memory(const unsigned char *buffer, size_t len,
                                                 int *x, int *y, int *comp, int req_comp)
{
    if (len < 8 || png_sig_cmp((png_bytep)buffer, 0, 8))
    {
        stbi__set_failure("PNG signature not found");
        return NULL;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        stbi__set_failure("Failed to create png_struct");
        return NULL;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        stbi__set_failure("Failed to create png_info");
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        stbi__set_failure("PNG decode error");
        return NULL;
    }

    stbi__png_stream stream = { buffer, len, 0 };
    png_set_read_fn(png_ptr, &stream, stbi__png_read);
    png_set_sig_bytes(png_ptr, 0);

    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    int desired_channels = req_comp ? req_comp : 0;
    if (desired_channels == 4)
    {
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }
    else if (desired_channels == 3)
    {
        if (color_type == PNG_COLOR_TYPE_RGBA || color_type == PNG_COLOR_TYPE_GA)
            png_set_strip_alpha(png_ptr);
    }

    png_read_update_info(png_ptr, info_ptr);

    png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    int channels = (int)png_get_channels(png_ptr, info_ptr);
    int final_channels = desired_channels ? desired_channels : channels;

    unsigned char *image_data = (unsigned char *)malloc(rowbytes * height);
    if (!image_data)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        stbi__set_failure("PNG out of memory");
        return NULL;
    }

    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    if (!row_pointers)
    {
        free(image_data);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        stbi__set_failure("PNG out of memory");
        return NULL;
    }

    for (png_uint_32 i = 0; i < height; ++i)
    {
        png_uint_32 row_index = stbi__flip_vertically ? (height - 1 - i) : i;
        row_pointers[i] = image_data + row_index * rowbytes;
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(row_pointers);

    if (x) *x = (int)width;
    if (y) *y = (int)height;
    if (comp) *comp = final_channels;

    // If we asked for a specific number of channels, libpng already handled it
    return image_data;
}

// ----------------------- JPEG loader helpers -----------------------
static unsigned char *stbi__jpeg_load_from_memory(const unsigned char *buffer, size_t len,
                                                  int *x, int *y, int *comp, int req_comp)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr
    {
        struct jpeg_error_mgr pub;
        jmp_buf setjmp_buffer;
    } jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    cinfo.err->error_exit = [](j_common_ptr cinfo_ptr)
    {
        my_error_mgr *myerr = (my_error_mgr *)cinfo_ptr->err;
        longjmp(myerr->setjmp_buffer, 1);
    };

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        stbi__set_failure("JPEG decode error");
        return NULL;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, buffer, len);

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
    {
        jpeg_destroy_decompress(&cinfo);
        stbi__set_failure("Invalid JPEG header");
        return NULL;
    }

    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    int width = (int)cinfo.output_width;
    int height = (int)cinfo.output_height;
    int channels = (int)cinfo.output_components; // typically 3

    int desired_channels = req_comp ? req_comp : channels;
    int row_stride = width * channels;
    unsigned char *image_data = (unsigned char *)malloc((size_t)width * (size_t)height * (size_t)desired_channels);
    if (!image_data)
    {
        jpeg_destroy_decompress(&cinfo);
        stbi__set_failure("JPEG out of memory");
        return NULL;
    }

    unsigned char *temp_row = (unsigned char *)malloc((size_t)row_stride);
    if (!temp_row)
    {
        free(image_data);
        jpeg_destroy_decompress(&cinfo);
        stbi__set_failure("JPEG out of memory");
        return NULL;
    }

    while (cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *rowptr = image_data;
        unsigned int scanline = cinfo.output_scanline;
        if (stbi__flip_vertically)
            rowptr += (height - 1 - scanline) * width * desired_channels;
        else
            rowptr += scanline * width * desired_channels;

        JSAMPROW buffer_array[1];
        buffer_array[0] = temp_row;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);

        for (int i = 0; i < width; ++i)
        {
            unsigned char r = temp_row[i * channels + 0];
            unsigned char g = (channels > 1) ? temp_row[i * channels + 1] : r;
            unsigned char b = (channels > 2) ? temp_row[i * channels + 2] : r;

            rowptr[i * desired_channels + 0] = r;
            if (desired_channels > 1)
            {
                rowptr[i * desired_channels + 1] = g;
                rowptr[i * desired_channels + 2] = b;
                if (desired_channels == 4)
                    rowptr[i * desired_channels + 3] = 255;
            }
        }
    }

    free(temp_row);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    if (x) *x = width;
    if (y) *y = height;
    if (comp) *comp = desired_channels;
    return image_data;
}

// ----------------------- Public API -----------------------
unsigned char *stbi_load_from_memory(const unsigned char *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
    if (len <= 0 || buffer == NULL)
    {
        stbi__set_failure("Invalid buffer");
        return NULL;
    }

    // Check PNG signature
    if (len >= 8 && !png_sig_cmp((png_bytep)buffer, 0, 8))
    {
        return stbi__png_load_from_memory(buffer, (size_t)len, x, y, comp, req_comp);
    }

    // Check JPEG signature (0xFFD8)
    if (len >= 2 && buffer[0] == 0xFF && buffer[1] == 0xD8)
    {
        return stbi__jpeg_load_from_memory(buffer, (size_t)len, x, y, comp, req_comp);
    }

    stbi__set_failure("Unknown or unsupported format");
    return NULL;
}

unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        stbi__set_failure("Failed to open file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length <= 0)
    {
        fclose(f);
        stbi__set_failure("Empty file");
        return NULL;
    }

    unsigned char *buffer = (unsigned char *)malloc((size_t)length);
    if (!buffer)
    {
        fclose(f);
        stbi__set_failure("Out of memory");
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)length, f);
    fclose(f);
    if (read != (size_t)length)
    {
        free(buffer);
        stbi__set_failure("Failed to read file");
        return NULL;
    }

    unsigned char *result = stbi_load_from_memory(buffer, (int)length, x, y, comp, req_comp);
    free(buffer);
    return result;
}

#endif // STB_IMAGE_IMPLEMENTATION
