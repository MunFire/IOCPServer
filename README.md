c++사용한 IOCP 간단한 서버입니다.
flatbuffer, mysql, redis를 사용했습니다.

flatbuffer사용해서 계정생성, 로그인, 프로필정보 패킷을 만들었고
redis를 사용하여 세션관리와 계정의 프로필 데이터캐싱과 로그인실패시횟수를 체크합니다.
계정정보를 mysql로 간단히 저장하는 프로그램입니다.

IOCPServer.cpp – 앱 진입점 및 IOCP 포트 생성·스레드 관리
Client.cpp/Client.h – 클라이언트 소켓 핸들링, 비동기 I/O 요청(PostRecv/PostSend)
DBManager.cpp/DBManager.h, DBWorker.cpp/DBWorker.h, DBTask.h – MySQL 연산을 위한 작업 큐·스레드 풀
RedisManager.cpp/RedisManager.h – 세션·캐싱을 위한 Redis 래퍼
auth.fbs – FlatBuffers 스키마 정의 (로그인/회원가입 메시지 포맷)
