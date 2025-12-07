#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <atomic>

namespace
{
    ma_engine g_engine{};
    ma_sound  g_music{};
    std::atomic<bool> g_engine_initialized{false};
    std::atomic<bool> g_music_initialized{false};
}

namespace audio
{
    bool init()
    {
        if (g_engine_initialized.load())
        {
            return true;
        }

        if (ma_engine_init(nullptr, &g_engine) != MA_SUCCESS)
        {
            return false;
        }

        g_engine_initialized.store(true);
        return true;
    }

    void play_music(const char* path, bool loop)
    {
        if (!g_engine_initialized.load())
        {
            return;
        }

        if (g_music_initialized.load())
        {
            ma_sound_uninit(&g_music);
            g_music_initialized.store(false);
        }

        ma_result result = ma_sound_init_from_file(
            &g_engine,
            path,
            MA_SOUND_FLAG_STREAM,
            nullptr,
            nullptr,
            &g_music);

        if (result != MA_SUCCESS)
        {
            return;
        }

        ma_sound_set_looping(&g_music, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(&g_music);
        g_music_initialized.store(true);
    }

    void shutdown()
    {
        if (g_music_initialized.load())
        {
            ma_sound_uninit(&g_music);
            g_music_initialized.store(false);
        }

        if (g_engine_initialized.load())
        {
            ma_engine_uninit(&g_engine);
            g_engine_initialized.store(false);
        }
    }
}
