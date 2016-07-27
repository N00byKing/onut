#pragma once
// Onut
#include <onut/Resource.h>
#include <onut/Timer.h>

// STL
#include <atomic>
#include <vector>

// Forward
#include <onut/ForwardDeclaration.h>
OForwardDeclare(AudioStream);
OForwardDeclare(ContentManager);
OForwardDeclare(Sound);
OForwardDeclare(SoundCue);
OForwardDeclare(SoundInstance);

namespace onut
{
    class AudioStream
    {
    protected:
        friend class AudioEngine;

        // Returns false when completed
        virtual bool progress(int frameCount, int channelCount, float* pOut) = 0;
    };

    class SoundInstance final : public AudioStream, public std::enable_shared_from_this<AudioStream>
    {
    public:
        void play();
        void pause();
        void stop();

        bool isPlaying() const;
        bool isPaused() const;

        bool getLoop() const;
        void setLoop(bool loop);

        float getVolume() const;
        void setVolume(float volume);

        float getBalance() const;
        void setBalance(float balance);

        float getPitch() const;
        void setPitch(float pitch);

        ~SoundInstance();

    private:
        friend class Sound;
        bool progress(int frameCount, int channelCount, float* pOut) override;

        bool m_isPaused = true;
        std::atomic<bool> m_loop = false;
        std::atomic<float> m_volume = 1.f;
        std::atomic<float> m_balance = 0.f;
        std::atomic<float> m_pitch = 1.f;
        std::atomic<int> m_offset = 0;
        OSoundRef m_pSound;
    };

    class Sound final : public Resource, public std::enable_shared_from_this<Sound>
    {
    public:
        static OSoundRef createFromFile(const std::string& filename, const OContentManagerRef& pContentManager = nullptr);
        static OSoundRef createFromData(float* pSamples, int sampleCount, int channelCount, int samplerate, const OContentManagerRef& pContentManager = nullptr);

        ~Sound();

        void setMaxInstance(int maxInstance = -1) { m_maxInstance = maxInstance; }
        void play(float volume = 1.f, float balance = 0.f, float pitch = 1.f);
        void stop();

        OSoundInstanceRef createInstance();

    private:
        friend class SoundInstance;
        friend class AudioEngine;

        using Instances = std::vector<OSoundInstanceRef>;

        float* m_pBuffer = nullptr;
        int m_bufferSampleCount = 0;
        Instances m_instances;
        int m_maxInstance = -1;
    };
    
    class SoundCue final : public Resource
    {
    public:
        static OSoundCueRef createFromFile(const std::string& filename, const OContentManagerRef& pContentManager = nullptr);

        void play(float volume = 1.0f, float balance = 0.f, float pitch = 1.f);

    private:
        struct Sound
        {
            OSoundRef pSound;

            float volume = 1.f;
            float balance = 0.f;
            float pitch = 1.f;
            float delay = 0.f;
            float chance = 0.f;
        };

        struct Play
        {
            std::vector<Sound> sounds;
            float totalChances = 0.f;

            OTimer playTimer;
        };

        using Plays = std::vector<Play>;

        Plays m_plays;
    };
}

OSoundRef OGetSound(const std::string& name);
void OPlaySound(const std::string& name, float volume = 1.f, float balance = 0.f, float pitch = 1.f);
void OPlayRandomSound(const std::vector<std::string>& sounds, float volume = 1.0f, float balance = 0.f, float pitch = 1.f);
OSoundInstanceRef OCreateSoundInstance(const std::string& name);
OSoundCueRef OGetSoundCue(const std::string& name);
void OPlaySoundCue(const std::string& name, float volume = 1.f, float balance = 0.f, float pitch = 1.f);
