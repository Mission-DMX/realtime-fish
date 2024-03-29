#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "generator-tmp/ShowFile_v0.xml.hpp"

MissionDMX::ShowFile::Scene get_first_scene() {
	MissionDMX::ShowFile::Scene s;
	s.human_readable_name("An example scene containing all filters with an example.");
	s.id(0);
	std::vector<MissionDMX::ShowFile::Filter> filters;

	filters.emplace_back(0, "const_8bit", "");
	filters[0].initialParameters().push_back(MissionDMX::ShowFile::KeyValuePair("value", "50"));

	filters.emplace_back(1, "const_16bit", "");
	filters[1].initialParameters().push_back(MissionDMX::ShowFile::KeyValuePair("value", "9001"));

	filters.emplace_back(2, "const_float", "");
	filters[2].initialParameters().push_back(MissionDMX::ShowFile::KeyValuePair("value", "13.6"));

	filters.emplace_back(3, "const_color", "");
	filters[3].initialParameters().push_back(MissionDMX::ShowFile::KeyValuePair("value", "270.0,0.5,0.99"));

	filters.emplace_back(4, "debug_8bit", "");
	filters[4].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "const_8bit:value"));

	filters.emplace_back(5, "debug_16bit", "");
	filters[5].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "const_16bit:value"));

	filters.emplace_back(6, "debug_float", "");
	filters[6].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "multiply_add_filter:value"));

	filters.emplace_back(7, "debug_color", "");
	filters[7].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "const_color:value"));

	filters.emplace_back(8, "conversion_split_16bit", "");
	filters[8].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "const_16bit:value"));

	filters.emplace_back(9, "conversion_16bit_to_bool", "");
	filters[9].channellink().push_back(MissionDMX::ShowFile::ChannelLink("value", "const_16bit:value"));

	filters.emplace_back(10, "multiply_add_filter", "");
	filters[10].channellink().push_back(MissionDMX::ShowFile::ChannelLink("factor1", "const_float:value"));
	filters[10].channellink().push_back(MissionDMX::ShowFile::ChannelLink("factor2", "const_float:value"));
	filters[10].channellink().push_back(MissionDMX::ShowFile::ChannelLink("summand", "const_float:value"));

	filters.emplace_back(11, "universe_output", "");
	filters[11].filterConfiguration().push_back(::MissionDMX::ShowFile::KeyValuePair("1", "input_1"));
	filters[11].filterConfiguration().push_back(::MissionDMX::ShowFile::KeyValuePair("universe", "1"));
	filters[11].channellink().push_back(MissionDMX::ShowFile::ChannelLink("input_1", "const_8bit:value"));

	for(const auto& f : filters)
		s.filter().push_back(f);
	return s;
}

MissionDMX::ShowFile::Scene get_second_scene() {
	MissionDMX::ShowFile::Scene s;
	s.human_readable_name("A minimal example scene. It does not contain any filters.");
	s.id(1);
	return s;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Ussage: sample_xml_generator <sample file>" << std::endl;
		return 1;
	}
	try {
		MissionDMX::ShowFile::BordConfiguration obj;
		obj.show_name("A generated sample show file");
		obj.default_active_scene(0);
		obj.notes("This field may contain any human readable show notes");

		obj.scene().push_back(get_first_scene());
		obj.scene().push_back(get_second_scene());

		MissionDMX::ShowFile::Device d;
		d.id(0);
		d.channel(0);
		d.type("A device type. Keep in mind that fish does not use information about devices.");
		d.name("The name or description of a device");
		d.universe_id(0);
		obj.device().push_back(d);

		std::vector<MissionDMX::ShowFile::Universe> universes;
		MissionDMX::ShowFile::Patching patching_t;
		universes.emplace_back(patching_t, 0);
		universes[0].physical_location(0);
		universes[0].name("Universe 1");
		universes[0].description("A universe mapped to the first DMX port on the console. The description field can be used for the user to store notes or hints about the stage configuration. Please note that the ID of the universe is different from the physical address and a shared name space with other universe types in which it needs to be unique.");
		universes.emplace_back(patching_t, 1);
		universes[1].artnet_location(MissionDMX::ShowFile::ArtnetLocation("10.0.15.1", 6465, 0));
		universes[1].name("ArtNet Universe 1");
		universes[1].description("This universe would be mapped to an artnet device (such as a stage box or complex light controller). Keep in mind that physical universes and artnet configurations are mutually exclusive.");
		universes[1].id(1);
		for(const auto& u : universes)
			obj.universe().push_back(u);

		obj.uihint().push_back(MissionDMX::ShowFile::KeyValuePair("key", "value"));
		obj.uihint().push_back(MissionDMX::ShowFile::KeyValuePair("description", "This is a key value storage not uitilized by fish. The GUI may use this to store any data it may desire."));

		xml_schema::namespace_infomap imap;
		const auto cwd = std::filesystem::current_path().string();
		imap[""].schema = cwd + "/submodules/Docs/FormatSchemes/ProjectFile/ShowFile_v0.xsd";
		imap[""].name = "";

		std::ofstream filestream(argv[1]);
		MissionDMX::ShowFile::bord_configuration(filestream, obj, imap);
		std::cout << "Successfully generated " << argv[1] << "." << std::endl;
	} catch(const xml_schema::exception& e) {
		std::cerr << e << std::endl;
		return 1;
	}
	return 0;
}
