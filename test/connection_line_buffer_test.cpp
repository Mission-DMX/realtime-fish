#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>
//
// #include "net/socketaddress.hpp"
// #include "net/sock_address_factory.hpp"
#include <iostream>


// using namespace rmrf::net;
//
BOOST_AUTO_TEST_CASE(helloworld) {
	std::cout << "helloworld" << std::endl;
	BOOST_CHECK_EQUAL("abc", "abc");
}

// BOOST_AUTO_TEST_CASE(Socketaddr_IPv4_Construction_Test) {
// 	sockaddr_in addr_ip4;
// 	addr_ip4.sin_family = AF_INET;
// 	addr_ip4.sin_addr.s_addr = htonl(INADDR_ANY);
// 	addr_ip4.sin_port = htons(80);
// 	socketaddr sa{addr_ip4};
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv4 0.0.0.0:80");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_Lookup_Localhost_IPv6) {
// 	const auto sa = get_first_general_socketaddr("::1", 443);
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv6 [::1]:443");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_Lookup_Localhost_through_indirect_IPv6) {
// 	const auto sa = get_first_general_socketaddr("[::1]", 443);
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv6 [::1]:443");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_Lookup_Localhost_through_direct_IPv6) {
// 	const auto sa = get_first_general_socketaddr("::1", 443);
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv6 [::1]:443");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_Lookup_Localhost_through_IPv4) {
// 	const auto sa = get_first_general_socketaddr("127.0.0.1", "443");
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv4 127.0.0.1:443");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_IPv6_Construction_Test) {
// 	sockaddr_in6 addr_ip6;
// 	addr_ip6.sin6_family = AF_INET6;
// 	addr_ip6.sin6_addr = in6addr_any;
// 	addr_ip6.sin6_port = htons(80);
// 	socketaddr sa{addr_ip6};
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv6 [::]:80");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_IPv6_Localhost_Construction_Test) {
// 	sockaddr_in6 addr_ip6;
// 	addr_ip6.sin6_family = AF_INET6;
// 	addr_ip6.sin6_addr = in6addr_loopback;
// 	addr_ip6.sin6_port = htons(80);
// 	socketaddr sa{addr_ip6};
// 	BOOST_CHECK_EQUAL(sa.str(), "SocketAddress: IPv6 [::1]:80");
// }
//
// BOOST_AUTO_TEST_CASE(Socketaddr_comparison) {
// 	BOOST_CHECK_NE(get_first_general_socketaddr("127.0.0.1", "443"), get_first_general_socketaddr("127.0.0.1", "80"));
// 	BOOST_CHECK_EQUAL(get_first_general_socketaddr("127.0.0.1", "443"), get_first_general_socketaddr("127.0.0.1", "443"));
// 	BOOST_CHECK_NE(get_first_general_socketaddr("::1", "443"), get_first_general_socketaddr("127.0.0.1", "80"));
// 	BOOST_CHECK_NE(get_first_general_socketaddr("[::1]", "443"), get_first_general_socketaddr("::1", "80"));
// 	BOOST_CHECK_EQUAL(get_first_general_socketaddr("[::1]", "443"), get_first_general_socketaddr("::1", "443"));
// }
