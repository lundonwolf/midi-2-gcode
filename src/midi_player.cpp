#include "midi_player.h"
#include <chrono>
#include <iostream>
#include "porttime.h"

MidiPlayer::MidiPlayer()
    : m_stream(nullptr)
    , m_isPlaying(false)
    , m_shouldStop(false)
    , m_tempo(1.0f)
    , m_startTime(0)
    , m_currentEventIndex(0)
{
}

MidiPlayer::~MidiPlayer() {
    cleanup();
}

bool MidiPlayer::initialize() {
    PmError pmErr = Pm_Initialize();
    if (pmErr != pmNoError) {
        std::cerr << "Failed to initialize PortMidi: " << Pm_GetErrorText(pmErr) << std::endl;
        return false;
    }
    
    PtError ptErr = Pt_Start(1, nullptr, nullptr);
    if (ptErr != ptNoError) {
        std::cerr << "Failed to initialize PortTime" << std::endl;
        Pm_Terminate();
        return false;
    }
    
    return true;
}

void MidiPlayer::cleanup() {
    stop();
    if (m_stream) {
        Pm_Close(m_stream);
        m_stream = nullptr;
    }
    Pt_Stop();
    Pm_Terminate();
}

bool MidiPlayer::loadMidiFile(const std::string& filename) {
    // TODO: Implement MIDI file parsing
    // For now, just create a test sequence
    m_events.clear();
    
    // Example test sequence
    MidiEvent evt;
    evt.timestamp = 0;
    evt.status = 0x90;  // Note On, channel 0
    evt.data1 = 60;     // Middle C
    evt.data2 = 100;    // Velocity
    m_events.push_back(evt);
    
    evt.timestamp = 500;
    evt.status = 0x80;  // Note Off, channel 0
    evt.data1 = 60;
    evt.data2 = 0;
    m_events.push_back(evt);
    
    return true;
}

void MidiPlayer::play() {
    if (m_isPlaying || m_events.empty()) return;
    
    m_shouldStop = false;
    m_isPlaying = true;
    m_currentEventIndex = 0;
    m_startTime = Pt_Time();
    
    m_playbackThread = std::make_unique<std::thread>(&MidiPlayer::playbackThread, this);
}

void MidiPlayer::pause() {
    m_isPlaying = false;
    if (m_playbackThread && m_playbackThread->joinable()) {
        m_playbackThread->join();
    }
}

void MidiPlayer::stop() {
    m_shouldStop = true;
    m_isPlaying = false;
    if (m_playbackThread && m_playbackThread->joinable()) {
        m_playbackThread->join();
    }
    m_currentEventIndex = 0;
}

void MidiPlayer::setTempo(float tempo) {
    m_tempo = tempo;
}

float MidiPlayer::getPlaybackPosition() const {
    if (!m_isPlaying) return 0.0f;
    return static_cast<float>(Pt_Time() - m_startTime) / 1000.0f;
}

bool MidiPlayer::isPlaying() const {
    return m_isPlaying;
}

std::vector<std::string> MidiPlayer::getAvailableOutputDevices() const {
    std::vector<std::string> devices;
    int numDevices = Pm_CountDevices();
    
    for (int i = 0; i < numDevices; i++) {
        const PmDeviceInfo* info = Pm_GetDeviceInfo(i);
        if (info->output) {
            devices.push_back(info->name);
        }
    }
    
    return devices;
}

bool MidiPlayer::setOutputDevice(int deviceIndex) {
    if (m_stream) {
        Pm_Close(m_stream);
        m_stream = nullptr;
    }
    
    PmError err = Pm_OpenOutput(&m_stream, deviceIndex, nullptr, 0, nullptr, nullptr, 0);
    return err == pmNoError;
}

void MidiPlayer::playbackThread() {
    while (m_isPlaying && !m_shouldStop && m_currentEventIndex < m_events.size()) {
        const MidiEvent& evt = m_events[m_currentEventIndex];
        long currentTime = Pt_Time() - m_startTime;
        
        if (currentTime >= evt.timestamp) {
            if (m_stream) {
                PmEvent pmEvt;
                pmEvt.message = Pm_Message(evt.status, evt.data1, evt.data2);
                pmEvt.timestamp = 0;
                Pm_Write(m_stream, &pmEvt, 1);
            }
            m_currentEventIndex++;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    m_isPlaying = false;
}
