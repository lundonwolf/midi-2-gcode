#pragma once

#include <string>
#include <vector>
#include <portmidi.h>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

class MidiPlayer {
public:
    MidiPlayer();
    ~MidiPlayer();

    bool initialize();
    bool loadMidiFile(const std::string& filename);
    void play();
    void pause();
    void stop();
    void setTempo(float tempo);
    float getPlaybackPosition() const;
    bool isPlaying() const;
    std::vector<std::string> getAvailableOutputDevices() const;
    bool setOutputDevice(int deviceIndex);

private:
    struct MidiEvent {
        long timestamp;  // in milliseconds
        unsigned char status;
        unsigned char data1;
        unsigned char data2;
    };

    void playbackThread();
    void cleanup();

    std::vector<MidiEvent> m_events;
    PortMidiStream* m_stream;
    std::unique_ptr<std::thread> m_playbackThread;
    std::mutex m_mutex;
    std::atomic<bool> m_isPlaying;
    std::atomic<bool> m_shouldStop;
    float m_tempo;
    long m_startTime;
    size_t m_currentEventIndex;
};
