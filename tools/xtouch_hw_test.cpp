/**
* This tools can be used to test attached xtouch devices for functionality.
*/

#include <iostream>
#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "control_desk/device_enumeration.hpp"
#include "control_desk/xtouch_driver.hpp"
#include "control_desk/device_handle.hpp"

using namespace dmxfish::control_desk;

/* ------------------------------------------------------------------ */
/* 1️⃣  Stub for device discovery – fill in your own implementation     */
/* ------------------------------------------------------------------ */
std::list<std::shared_ptr<device_handle>> list_device_handles()
{
    // ---------------------------------------------------------------
    // TODO: Open the MIDI/USB device(s) that represent the X‑Touch,
    //       push them into the list and return it.
    // Example (pseudo‑code):
    //   auto h = std::make_shared<device_handle>();
    //   h->open_by_name("X‑Touch Mini");
    //   devices.push_back(h);
    // ---------------------------------------------------------------
    return {};   // <- replace with the real list
}

/* ------------------------------------------------------------------ */
/* 2️⃣  Small wrapper that blocks until a matching event arrives        */
/* ------------------------------------------------------------------ */
struct midi_event_matcher {
    std::atomic<bool>         ready{false};
    std::mutex                mtx;
    std::condition_variable   cv;
    midi_command              expected{};

    // called from the async receive‑callback of device_handle
    void on_message(const midi_command& msg)
    {
        if (msg == expected) {
            ready = true;
            cv.notify_one();
        }
    }

    bool wait_for(std::chrono::seconds timeout = std::chrono::seconds{10})
    {
        std::unique_lock lk{mtx};
        return cv.wait_for(lk, timeout, [this]{ return ready.load(); });
    }
};

/* ------------------------------------------------------------------ */
/* 3️⃣  Register a global callback for each opened device               */
/* ------------------------------------------------------------------ */
void attach_listener(const std::shared_ptr<device_handle>& dev,
                     midi_event_matcher& matcher)
{
    dev->set_receive_callback(
            [&matcher](const midi_command& msg){ matcher.on_message(msg); });
}

/* ------------------------------------------------------------------ */
/* 4️⃣  Helper to pause for a key press                               */
/* ------------------------------------------------------------------ */
void press_enter(const std::string& prompt)
{
    std::cout << prompt << "  Press <Enter> to continue…";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

/* ------------------------------------------------------------------ */
/* 5️⃣  Simple colour set for readability                              */
/* ------------------------------------------------------------------ */
void colourize(const char* txt, const char* colour_code = "\033[1;32m")
{
    std::cout << colour_code << txt << "\033[0m";
}

void test_leds(const std::list<std::shared_ptr<device_handle>>& devices)
{
    std::cout << "\n=== LED TEST ===\n";

    // Light all button LEDs (off → on)
    for (auto b : xtouch_buttons{}) {
        for (auto& dev : devices)
            xtouch_set_button_led(*dev, b, button_led_state::on);
    }

    // Light all encoder rings (value 127 = full)
    for (auto e : xtouch_faders{}) {               // reuse iterator – same range
        for (auto& dev : devices)
            xtouch_set_ring_led(*dev,
                                static_cast<encoder>(static_cast<uint8_t>(e) - XTOUCH_ENCODER_INDEX_OFFSET + 80),
                                127);
    }

    // Light all meter bars (value 127 = all LEDs)
    for (auto l : xtouch_faders{}) {               // reuse iterator again
        for (auto& dev : devices)
            xtouch_set_meter_leds(*dev,
                                  static_cast<led_bar>(static_cast<uint8_t>(l) - XTOUCH_FADER_INDEX_OFFSET + 90),
                                  127);
    }

    press_enter("All LEDs should now be fully on.");
}

void test_buttons(const std::list<std::shared_ptr<device_handle>>& devices)
{
    std::cout << "\n=== BUTTON TEST ===\n";

    midi_event_matcher matcher;
    for (auto& dev : devices) attach_listener(dev, matcher);

    for (auto b : xtouch_buttons{}) {
        // Expected MIDI NOTE_ON with velocity = 127 (press)
        matcher.expected = midi_command{midi_status::NOTE_ON, 0, static_cast<uint8_t>(b), 127};
        matcher.ready = false;

        std::cout << "Press button " << static_cast<int>(b) << "... ";
        if (!matcher.wait_for()) {
            colourize("\n[FAIL] No event received.\n", "\033[1;31m");
        } else {
            colourize("\n[OK]\n", "\033[1;32m");
        }

        // also wait for release (velocity 0) so the next press is clean
        matcher.expected = midi_command{midi_status::NOTE_ON, 0, static_cast<uint8_t>(b), 0};
        matcher.ready = false;
        matcher.wait_for();   // ignore timeout – a missing release is not fatal
    }
}

void test_fader_manual(const std::list<std::shared_ptr<device_handle>>& devices)
{
    std::cout << "\n=== MANUAL FADER TEST (0 → 127) ===\n";

    midi_event_matcher matcher;
    for (auto& dev : devices) attach_listener(dev, matcher);

    for (auto f : xtouch_faders{}) {
        std::cout << "Slide fader " << static_cast<int>(f)
                  << " from 0 to the top position and release it.\n";
        // Expect a CONTROL_CHANGE with any value > 0 (press)
        matcher.expected = midi_command{midi_status::CONTROL_CHANGE, 0,
                                        static_cast<uint8_t>(f), 0};
        matcher.ready = false;

        // Wait for the first non‑zero value (the user moving the fader)
        while (true) {
            if (!matcher.wait_for(std::chrono::seconds{20})) {
                colourize("[TIMEOUT] No movement detected.\n", "\033[1;31m");
                break;
            }
            // a non‑zero value was received – accept it
            if (matcher.expected.velocity != 0) break; // TODO
        }
    }
}

void test_fader_auto(const std::list<std::shared_ptr<device_handle>>& devices)
{
    std::cout << "\n=== AUTOMATIC FADER SWEEP (0 → 127 → 0) ===\n";

    for (auto f : xtouch_faders{}) {
        std::cout << "Sweeping fader " << static_cast<int>(f) << " …\n";

        // 0 → 127
        for (uint8_t pos = 0; pos <= 127; ++pos) {
            for (auto& dev : devices)
                xtouch_set_fader_position(*dev, f, pos);
            std::this_thread::sleep_for(std::chrono::milliseconds{5});
        }

        // 127 → 0
        for (int pos = 127; pos >= 0; --pos) {
            for (auto& dev : devices)
                xtouch_set_fader_position(*dev, f, static_cast<uint8_t>(pos));
            std::this_thread::sleep_for(std::chrono::milliseconds{5});
        }

        press_enter("Did the fader follow the sweep correctly?");
    }
}

int main()
{
    // ----------------------------------------------------------------
    // 1️⃣  Discover devices
    // ----------------------------------------------------------------
    auto devices = list_device_handles();
    if (devices.empty()) {
        std::cerr << "No X‑Touch devices found – aborting.\n";
        return 1;
    }
    std::cout << "Found " << devices.size() << " device(s).\n";

    // ----------------------------------------------------------------
    // 2️⃣  Run the individual tests
    // ----------------------------------------------------------------
    test_leds(devices);
    test_buttons(devices);
    test_fader_manual(devices);
    test_fader_auto(devices);

    std::cout << "\nAll tests finished.\n";
    return 0;
}
