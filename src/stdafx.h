// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"


#include <stdio.h>
#include <tchar.h>

#include <cassert>
#include <cstdlib>
// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы



#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>



#include <fstream>
#include <iostream>
#include <sstream>


#include <algorithm>
#include <complex>
#include <deque>
#include <exception>
#include <functional>
#include <initializer_list>

#include <iterator>
#include <limits>
#include <locale>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <ratio>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>      // std::rel_ops



////RTTI
#include <typeindex>
#include <typeinfo>
#include <type_traits>


////Multi-threading
#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>


///C++ 14/17
#include <experimental/filesystem>





//using namespace std::rel_ops;

#include <process.h>

///Windows


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//Third party

#include "ini.h"


///My

using namespace std;
namespace fs = std::experimental::filesystem;

#include "Exceptions.h"