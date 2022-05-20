#pragma once

namespace patch::fast {
	inline class fast_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast";

	public:
		void init() {
            enabled_i = enabled;
        }

        void switching(bool flag) {
            enabled = flag;
        }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

	} fast;
}
