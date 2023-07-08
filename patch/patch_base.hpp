/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

namespace patch {
	class patch_base1 {

	protected:
		bool enabled;

	public:
		void switch_true_to_false() {}

		void switch_false_to_true() {}

		void switch_enabled(bool flag) {
			if (flag) {
				if (!enabled) switch_false_to_true();
			}
			else {
				if (enabled) switch_true_to_false();
			}
		}

	};

	class patch_base2 : public patch_base1 {

	protected:
		bool enabled_i;

	public:
		void switch_enabled(bool flag) {
			enabled = flag;
		}

		void init(bool flag) {
			enabled = enabled_i = flag;
		}
	};


}
