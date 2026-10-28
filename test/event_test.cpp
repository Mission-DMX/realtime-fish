//
// Created by doralitze on 12/14/23.
//
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "events/event_source.hpp"
#include "events/sender_parsing.hpp"
#include "main.hpp"

#include "../test/iomanager_test_fixture.hpp"

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

BOOST_AUTO_TEST_CASE(storage_is_sorted_by_sender_then_function_after_swap) {
    using namespace dmxfish::events;
    auto estorage = std::make_shared<event_storage>();
    auto src_a = event_source::create<event_source>(estorage);
    auto src_b = event_source::create<event_source>(estorage);
    auto src_c = event_source::create<event_source>(estorage);

    const auto a = src_a->get_sender_id();
    const auto b = src_b->get_sender_id();
    const auto c = src_c->get_sender_id();

    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{c, 2}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 5}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{b, 0}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 1}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 3}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{b, 7}}));

    estorage->swap_buffers();

    const auto& s = estorage->get_storage();
    BOOST_REQUIRE_EQUAL(s.size(), 6);
    for (size_t i = 1; i < s.size(); i++) {
        const auto& prev = s[i - 1].get_event_sender().decoded_representation;
        const auto& cur = s[i].get_event_sender().decoded_representation;
        const bool ordered = (prev.sender < cur.sender) ||
                             (prev.sender == cur.sender && prev.sender_function <= cur.sender_function);
        BOOST_CHECK(ordered);
    }
}

BOOST_AUTO_TEST_CASE(find_by_sender_returns_contiguous_matches) {
    using namespace dmxfish::events;
    auto estorage = std::make_shared<event_storage>();
    auto src_a = event_source::create<event_source>(estorage);
    auto src_b = event_source::create<event_source>(estorage);
    const auto a = src_a->get_sender_id();
    const auto b = src_b->get_sender_id();

    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 0}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{b, 1}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 2}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{a, 4}}));

    estorage->swap_buffers();

    const auto matches = estorage->find_by_sender(a);
    BOOST_CHECK_EQUAL(matches.size(), 3);
    for (const auto& ev : matches) {
        BOOST_CHECK_EQUAL(ev.get_event_sender().decoded_representation.sender, a);
    }

    const auto other = estorage->find_by_sender(b);
    BOOST_CHECK_EQUAL(other.size(), 1);

    const auto none = estorage->find_by_sender(99);
    BOOST_CHECK(none.empty());
}

BOOST_AUTO_TEST_CASE(find_exact_match_returns_matching_span) {
    using namespace dmxfish::events;
    auto estorage = std::make_shared<event_storage>();
    auto src = event_source::create<event_source>(estorage);
    const auto id = src->get_sender_id();

    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{id, 0}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{id, 1}}));
    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{id, 1}}));

    estorage->swap_buffers();

    const event_sender_t target{id, 1};
    const auto matches = estorage->find(target);
    BOOST_CHECK_EQUAL(matches.size(), 2);
    for (const auto& ev : matches) {
        BOOST_CHECK(ev.get_event_sender() == target);
    }

    const auto no_match = estorage->find(event_sender_t{id, 99});
    BOOST_CHECK(no_match.empty());
}

BOOST_AUTO_TEST_CASE(find_first_returns_truthy_ref_on_match) {
    using namespace dmxfish::events;
    auto estorage = std::make_shared<event_storage>();
    auto src = event_source::create<event_source>(estorage);
    const auto id = src->get_sender_id();

    BOOST_CHECK(estorage->insert_event(event{event_type::SINGLE_TRIGGER, event_sender_t{id, 3}}));
    estorage->swap_buffers();

    const event_sender_t target{id, 3};
    const auto hit = estorage->find_first(target);
    BOOST_CHECK(static_cast<bool>(hit));
    BOOST_CHECK(hit->get_event_sender() == target);

    const event_sender_t miss_target{id, 4};
    const auto miss = estorage->find_first(miss_target);
    BOOST_CHECK(!static_cast<bool>(miss));
}

static_assert(!std::is_default_constructible_v<dmxfish::events::event_ref>,
              "event_ref must not be default-constructible");
static_assert(!std::is_copy_assignable_v<dmxfish::events::event_ref>,
              "event_ref must not be copy-assignable");
static_assert(!std::is_move_assignable_v<dmxfish::events::event_ref>,
              "event_ref must not be move-assignable");
static_assert(sizeof(dmxfish::events::event_ref) == sizeof(const dmxfish::events::event*),
              "event_ref must be the same size as a raw pointer");

BOOST_AUTO_TEST_CASE(parse_sender_numeric_form) {
    using namespace dmxfish::events;
    const auto es = parse_sender_representation("17:3");
    BOOST_CHECK_EQUAL(es.decoded_representation.sender, 17u);
    BOOST_CHECK_EQUAL(es.decoded_representation.sender_function, 3u);
}

BOOST_AUTO_TEST_CASE(parse_sender_by_name) {
    using namespace dmxfish::events;
    auto estorage = get_event_storage_instance();
    BOOST_REQUIRE(estorage != nullptr);
    auto src = event_source::create<event_source>(estorage, "test-src-parse");
    const auto expected_id = src->get_sender_id();

    const auto es = parse_sender_representation("test-src-parse:3");
    BOOST_CHECK_EQUAL(es.decoded_representation.sender, expected_id);
    BOOST_CHECK_EQUAL(es.decoded_representation.sender_function, 3u);
}

BOOST_AUTO_TEST_CASE(parse_sender_name_with_digits) {
    using namespace dmxfish::events;
    auto estorage = get_event_storage_instance();
    BOOST_REQUIRE(estorage != nullptr);
    auto src = event_source::create<event_source>(estorage, "audio-7");
    const auto expected_id = src->get_sender_id();

    const auto es = parse_sender_representation("audio-7:0");
    BOOST_CHECK_EQUAL(es.decoded_representation.sender, expected_id);
    BOOST_CHECK_EQUAL(es.decoded_representation.sender_function, 0u);
}

BOOST_AUTO_TEST_CASE(parse_sender_unknown_name_throws) {
    using namespace dmxfish::events;
    BOOST_CHECK_THROW(parse_sender_representation("no-such-sender:0"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(parse_sender_malformed_throws) {
    using namespace dmxfish::events;
    BOOST_CHECK_THROW(parse_sender_representation("17"), std::invalid_argument);
}