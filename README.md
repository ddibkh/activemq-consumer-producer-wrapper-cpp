
# activemq sample 빌드 환경
kernel 버전 : 2.6.18-419.el5PAE i686 (centos 5.11 32bit)   
gcc 버전 : 4.7.0   
dependency : libapr(libapr-1.3.12), libuuid(libuuid-1.0.3), openssl (0.9.8m 이상)   
   
   
## activemq cpp 라이브러리 빌드   
	- activemq CMS cpp 라이브러리 다운로드 (activemq-cpp-library-3.9.5)   
		https://activemq.apache.org/components/cms/download/   
   
	- requirment   
		autoconf    >= 2.61   
		automake    >= 1.10   
		libtool     >= 1.5.24   
		APR         >= 1.3*   
		CPPUnit     >= 1.10.2* ( 1.12.1 is recommended )   
		OpenSSL     >= 0.9.8m* ( 1.0.0 or higher is recommended, this is an optional dependency)

	- build   
		./configure --prefix=/home/ddibkh/activemqbin --with-openssl=$OpensslbuildPath   
		ex)    
		$>./configure --prefix=/home/build/activemqbin --with-openssl=/home/build/   opensslbuild   
		$>make   
		$>make install   
   
   
## activemq 브로커 설치 및 실행 (jre 1.8 버전 이상)   
> 패키지 다운로드 (apache-activemq-5.15.11)
>>http://activemq.apache.org/download-archives

>기동 및 중지
>>$ACTIVEMQ_HOME/bin/activemq [start | stop]

>브로커 모니터링 포트 (8161) 
>설정변경 ($ACTIVEMQ_HOME/bin/env)
>>특정 사용자 권한으로 실행
>>>ACTIVEMQ_USER="user"

>>자바 home 설정
>>>JAVA_HOME="$javahome"
	

## 큐 데이터 유실방지   
producer 에서는 PERSISTENT 로 PUSH 해야한다. (default)   
그래야 broker 가 재기동 되더라도 데이터 유실이 되지 않는다.   
setTimeToLive(milli second) 함수는 broker queue 에 저장된 메시지 expiretime 을 지정한다. (함수 timeout 이 아님)   
   
   
consumer 에서는 AUTO_ACKNOWLEDGE 가 아닌 CLIENT_ACKNOWLEDGE 모드로 세션을 생셩하고
메시지 처리가 끝나면 message->acknowledge() 를 호출해야 broker 에서 메시지가 삭제된다.
```cpp
session = connection->createSession(Session::CLIENT_ACKNOWLEDGE); 
/*
메시지 수신 및 메시지 처리
*/
message->acknowledge();
```

## 샘플 프로그램 빌드 및 실행   
1) shared 링크 및 빌드   
   * shared.zip 파일 압축해제   
   * Makefile 의   
   LFLAGS = -L./lib/shared -lactivemq-cpp -lapr-1 -luuid -lrt -lcrypt -lpthread -ldl   
   * lib/shared 디렉토리에서 아래 명령 수행.   
   ln -s libactivemq-cpp.so.19.0.5 libactivemq-cpp.so   
   ln -s libapr-1.so.0.3.12 libapr-1.so   
   ln -s libuuid.so.1.0.0 libuuid.so   
   * make clean && make   
   * 공유라이브러리 링크   
   libapr-1.so.0.3.12, libuuid.so.1.0.0, libactivemq-cpp.so.19.0.5 파일을 /usr/lib 에 복사 후 ldconfig 실행.   
   (libapr 라이브러리가 설치되어 있지 않거나 libapr-1.so.0.3.12 보다 버전이 낮다면 libapr 복사)   
   (libuuid 라이브러리가 설치되어 있다면 libuuid 는 복사 스킵)   
   [xxxxxxxxx]$ ldd amq_mt_pool   
	linux-gate.so.1 =>  (0x00d14000)   
	libactivemq-cpp.so.19 => /usr/lib/libactivemq-cpp.so.19 (0x00d15000)   
	libapr-1.so.0 => /usr/lib/libapr-1.so.0 (0x0047e000)   
	libuuid.so.1 => /lib/libuuid.so.1 (0x05d54000)   
	librt.so.1 => /lib/librt.so.1 (0x00473000)   
	libcrypt.so.1 => /lib/libcrypt.so.1 (0x06120000)   
	libpthread.so.0 => /lib/libpthread.so.0 (0x00457000)   
	libdl.so.2 => /lib/libdl.so.2 (0x0043b000)   
	libstdc++.so.6 => /usr/local/lib/libstdc++.so.6 (0x004e6000)   
	libm.so.6 => /lib/libm.so.6 (0x00410000)   
	libgcc_s.so.1 => /usr/local/lib/libgcc_s.so.1 (0x00cb8000)   
	libc.so.6 => /lib/libc.so.6 (0x002b2000)   
	/lib/ld-linux.so.2 (0x00293000)   
   
2) static 링크 및 빌드   
   * static.zip 파일 압축해제   
   * Makefile 의   
   LFLAGS = -L./lib/static -lactivemq-cpp -lapr-1 -luuid -lrt -lcrypt -lpthread -ldl   
   * make clean && make   
   * 공유라이브러리 링크   
   [xxxxxxxxx]$ ldd amq_mt_pool   
   linux-gate.so.1 =>  (0x0088b000)   
	librt.so.1 => /lib/librt.so.1 (0x00473000)   
	libcrypt.so.1 => /lib/libcrypt.so.1 (0x06120000)   
	libpthread.so.0 => /lib/libpthread.so.0 (0x00457000)   
	libdl.so.2 => /lib/libdl.so.2 (0x0043b000)   
	libstdc++.so.6 => /usr/local/lib/libstdc++.so.6 (0x001a0000)   
	libm.so.6 => /lib/libm.so.6 (0x00410000)   
	libgcc_s.so.1 => /usr/local/lib/libgcc_s.so.1 (0x00e63000)   
	libc.so.6 => /lib/libc.so.6 (0x002b2000)   
	/lib/ld-linux.so.2 (0x00293000)   
   
3) 실행   
	./amq_mt_pool [producer | consumer] $threadcount   
	./amq_mt_pool producer 50   
	: 50 개의 스레드로 10개씩 총 500 개의 메시지 produce   
	./amq_mt_pool consumer 10   
	: 10 개의 스레드로 메시지 consume   
   