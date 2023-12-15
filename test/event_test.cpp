//
// Created by doralitze on 12/14/23.
//
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include <sstream>

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "events/event_source.hpp"

BOOST_AUTO_TEST_CASE(default_constructor_yields_invalid_event) {
    using namespace dmxfish::events;
    event e{};
    BOOST_CHECK(!e.is_valid());
    std::stringstream ss;
    ss << e;
    BOOST_CHECK_EQUAL(ss.str(), "Invalid Event");
    BOOST_CHECK_EQUAL(e.get_event_id(), 0);
}

BOOST_AUTO_TEST_CASE(continous_event_insertion) {
    using namespace dmxfish::events;
    auto estorage = std::make_shared<event_storage>();
    auto tesender = event_source::create<event_source>(estorage);

    const uint32_t sid = tesender->get_sender_id();
    event_sender_t s1(sid, 0);
    event e1{event_type::SINGLE_TRIGGER, s1};
    event e2{event_type::START, event_sender_t{sid, 1}};
    event e3{event_type::RELEASE, event_sender_t{sid, 1}};
    BOOST_CHECK_EQUAL(e1.get_event_sender().decoded_representation.sender, sid);
    BOOST_CHECK(e1.get_event_sender() != e2.get_event_sender());
    BOOST_CHECK(e1.get_event_sender().is_same_sender(e2.get_event_sender()));
    BOOST_CHECK(e1.get_event_sender().is_same_sender(e3.get_event_sender()));
    BOOST_CHECK(e2.get_event_sender() == e3.get_event_sender());
    BOOST_CHECK(estorage->insert_event(e1));
    BOOST_CHECK(estorage->insert_event(e2));
    BOOST_CHECK_EQUAL(estorage->get_storage().size(), 0);
    estorage->swap_buffers();
    BOOST_CHECK_EQUAL(estorage->get_storage().size(), 3);
    estorage->swap_buffers();
    BOOST_CHECK_EQUAL(estorage->get_storage().size(), 1);
    for(auto& e : estorage->get_storage()) {
        BOOST_CHECK_EQUAL(e.get_type(), event_type::ONGOING_EVENT);
        BOOST_CHECK(e.is_valid());
    }
    BOOST_CHECK(estorage->insert_event(e3));
    estorage->swap_buffers();
    BOOST_CHECK_EQUAL(estorage->get_storage().size(), 1);
    for(auto& e : estorage->get_storage()) {
        BOOST_CHECK_EQUAL(e.get_type(), event_type::RELEASE);
        BOOST_CHECK(e.is_valid());
    }
    estorage->swap_buffers();
    BOOST_CHECK_EQUAL(estorage->get_storage().size(), 0);
}