//
// Created by Leon Dietrich on 20.01.25.
//
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS

#include <boost/test/included/unit_test.hpp>

#include <sstream>

#include "executioners/state_registry.hpp"

BOOST_AUTO_TEST_CASE(get_and_set_states) {
    using namespace dmxfish::execution::state_registry;
    set("TestKey123", "abc");
    auto reply = get("TestKey456");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
    reply = get("TestKey123");
    BOOST_CHECK_EQUAL(reply.value(), "abc");

    set(1, "TestKey123", "def");
    reply = get(1, "TestKey456");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
    reply = get(1, "TestKey123");
    BOOST_CHECK_EQUAL(reply.value(), "def");
    reply = get(0, "TestKey123");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
}

BOOST_AUTO_TEST_CASE(return_states_on_empty_message) {
    using namespace dmxfish::execution::state_registry;
    using namespace missiondmx::fish::ipcmessages;
    reset_state_registry();
    set("MSG1TestKeyA", "abc1");
    set(1, "MSG1TestKey", "def1");
    state_list msg;
    BOOST_CHECK_EQUAL(update_states_from_message(msg), true);
    auto item_count = 0;
    for(auto& [k,v] : msg.unspecific_states()) {
        BOOST_CHECK_EQUAL(k, "MSG1TestKeyA");
        BOOST_CHECK_EQUAL(v, "abc1");
        item_count++;
    }
    BOOST_CHECK_EQUAL(item_count, 1);
    item_count = 0;
    for(auto& kvs : msg.specific_states()) {
        item_count++;
        BOOST_CHECK_EQUAL(kvs.k(), "MSG1TestKey");
        BOOST_CHECK_EQUAL(kvs.scene_id(), 1);
        BOOST_CHECK_EQUAL(kvs.v(), "def1");
    }
    BOOST_CHECK_EQUAL(item_count, 1);
}

BOOST_AUTO_TEST_CASE(state_reset_method) {
    using namespace dmxfish::execution::state_registry;
    reset_state_registry();
    set("resetkey1", "something");
    set(2, "resetkey2", "something else");
    dump_state_to_logging();
    auto reply = get("resetkey1");
    BOOST_CHECK_EQUAL(reply.has_value(), true);
    reply = get(2, "resetkey2");
    BOOST_CHECK_EQUAL(reply.has_value(), true);
    reset_state_registry();
    reply = get("resetkey1");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
    reply = get(2, "resetkey2");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
}

BOOST_AUTO_TEST_CASE(fill_states_on_message) {
    using namespace dmxfish::execution::state_registry;
    using namespace missiondmx::fish::ipcmessages;
    reset_state_registry();
    auto reply = get("fillkey1");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
    reply = get(2, "fillkey2");
    BOOST_CHECK_EQUAL(reply.has_value(), false);
    state_list msg;
    (*msg.mutable_unspecific_states())["fillkey1"] = "ghi";
    auto* specific_item = msg.add_specific_states();
    specific_item->set_k("fillkey2");
    specific_item->set_v("jkl");
    specific_item->set_scene_id(2);
    BOOST_CHECK_EQUAL(update_states_from_message(msg), false);
    reply = get("fillkey1");
    BOOST_CHECK_EQUAL(reply.value(), "ghi");
    reply = get(2, "fillkey2");
    BOOST_CHECK_EQUAL(reply.value(), "jkl");
}
