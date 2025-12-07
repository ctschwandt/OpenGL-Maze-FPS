#pragma once

namespace audio
{
    // Initialize the audio engine (returns false on failure).
    bool init();

    // Play a music file from disk, optionally looping.
    void play_music(const char* path, bool loop = true);

    // Shut down the audio engine and free resources.
    void shutdown();
}
