#pragma once

#include <memory>
#include <thread>

#include <ev++.h>
#include <spdlog-inl.h>

namespace dmxfish::io {

	class IOManager : public std::enable_shared_from_this<IOManager> {
		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<::ev::loop_ref> loop;
		public:
			IOManager(bool is_default_manager = false);
			~IOManager();
			void start{};
		private:
			void run();
	}
}
