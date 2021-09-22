#pragma once

#include <cstdio>

#define AMP_LOG_DBG(fmt, ...) fprintf(stdout, "\033[1;33m[DBG]:\033[0m " fmt "\n", ##__VA_ARGS__)
#define AMP_LOG_INF(fmt, ...) fprintf(stdout, "\033[1;32m[INF]:\033[0m " fmt "\n", ##__VA_ARGS__)
#define AMP_LOG_ERR(fmt, ...) fprintf(stderr, "\033[1;31m[ERR]:\033[0m " fmt "\n", ##__VA_ARGS__)
