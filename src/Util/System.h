//
//  System.hpp
//  ax_gen
//
//  Created by Jason on 2017-04-05.
//  Copyright © 2017 Jason. All rights reserved.
//

#pragma once

namespace ax_gen {

class System {
public:
	static void shellOpen(const StrView& path);
	
	static void createProcess(const StrView& exe, const StrView& args);

private:
	System() = delete;
};
	
} //namespace
