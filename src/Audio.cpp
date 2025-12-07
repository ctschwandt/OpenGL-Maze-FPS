#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <iostream>

namespace
{
    ma_engine g_engine{};
    bool g_initialized = false;

    ma_sound g_music{};
    bool g_music_loaded = false;
}

namespace audio
{
    bool init()
    {
        if (g_initialized)
        {
            return true;
        }

        ma_result result = ma_engine_init(nullptr, &g_engine);
        if (result != MA_SUCCESS)
        {
            std::cerr << "Failed to initialize audio engine: " << result << std::endl;
            return false;
        }

        g_initialized = true;
        return true;
    }

    void play_music(const char* path, bool loop)
    {
        if (!g_initialized)
        {
            if (!init())
            {
                return;
            }
        }

        if (g_music_loaded)
        {
            ma_sound_uninit(&g_music);
            g_music_loaded = false;
        }

        ma_result result = ma_sound_init_from_file(&g_engine,
                                                   path,
                                                   MA_SOUND_FLAG_STREAM,
                                                   nullptr,
                                                   nullptr,
                                                   &g_music);
        if (result != MA_SUCCESS)
        {
            std::cerr << "Failed to load music: " << path
                      << " (error " << result << ")" << std::endl;
            return;
        }

        ma_sound_set_looping(&g_music, loop ? MA_TRUE : MA_FALSE);

        result = ma_sound_start(&g_music);
        if (result != MA_SUCCESS)
        {
            std::cerr << "Failed to start music playback: " << result << std::endl;
            ma_sound_uninit(&g_music);
            return;
        }

        g_music_loaded = true;
    }

    void shutdown()
    {
        if (g_music_loaded)
        {
            ma_sound_uninit(&g_music);
            g_music_loaded = false;
        }

        if (g_initialized)
        {
            ma_engine_uninit(&g_engine);
            g_initialized = false;
        }
    }
}
