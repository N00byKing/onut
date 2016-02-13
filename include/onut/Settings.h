#pragma once
#include "onut/Point.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "onut/ForwardDeclaration.h"
OForwardDeclare(Settings);

namespace onut
{
    class Settings
    {
    public:
        using Resolution = Point;
        using SettingKey = std::string;
        using SettingValue = std::string;

        Settings();
        virtual ~Settings();

        const Resolution& getResolution() const { return m_resolution; }
        void setResolution(const Resolution& resolution);

        const std::string& getGameName() const { return m_gameName; }
        void setGameName(const std::string& gameName);

        bool getIsResizableWindow() const { return m_isResizableWindow; }
        void setIsResizableWindow(bool isBorderLessFullscreen);

        bool getBorderlessFullscreen() const { return m_isBorderLessFullscreen; }
        void setBorderlessFullscreen(bool m_isBorderLessFullscreen);

        bool getIsFixedStep() const { return m_isFixedStep; }
        void setIsFixedStep(bool isFixedStep);

        bool getIsEditorMode() const { return m_isEditorMode; }
        void setIsEditorMode(bool isEditorMode);

        void setUserSettingDefault(const SettingKey& key, const SettingValue& value);
        void setUserSetting(const SettingKey& key, const SettingValue& value);
        const SettingValue& getUserSetting(const SettingKey& key);

    private:
        using UserSettings = std::unordered_map<SettingKey, SettingValue>;

        Resolution m_resolution = Resolution{800, 600};
        std::string m_gameName = "Game Name";
        bool m_isResizableWindow = false;
        bool m_isFixedStep = true;
        bool m_isBorderLessFullscreen = false;
        bool m_isEditorMode = false;
        UserSettings m_userSettings;

        std::atomic<bool> m_isDirty = false;
        std::atomic<bool> m_isRunning = false;
        std::thread m_savingThread;
        std::mutex m_mutex;
        std::condition_variable m_conditionVariable;
    };
}

extern OSettingsRef oSettings;
