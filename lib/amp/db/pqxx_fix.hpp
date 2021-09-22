#pragma once

// include this header before including pqxx headers; TODO: research about this issue - really wierd
// workaround for pqxx - wierd errors - keywords work normally in local files but clang doesn't compile pqxx headers -
// MSVC works :(

#define and &&
#define or ||
#define not !
