#pragma once

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <random>
#include <memory>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

#include "flatbuffers/flatbuffers.h"

#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>

#include "flatbuffers/util.h"


#include "./Generated/auth_generated.h"
#include "Client.h"
#include "DBWorker.h"

#define PORT 12345
#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096