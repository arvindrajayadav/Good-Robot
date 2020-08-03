#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <deque>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <ctime>
#include <sstream>
#include <random>
#include <limits.h>

#if defined(__APPLE__)
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#else
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#endif

#include <boost/filesystem.hpp>

enum ButtonAction { BUAC_IGNORE, BUAC_LCLICK, BUAC_RCLICK, BUAC_GRABBED };