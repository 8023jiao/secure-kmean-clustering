#include <iostream>
#include <thread>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <numeric>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <Tests/Tests.h>

#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>
#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <ivoryTests/Circuit_Tests.h>


#include "CLP.h"
#include "main.h"
using namespace osuCrypto;

void OT_100Receive_Test(BitVector& choiceBits, gsl::span<block> recv, gsl::span<std::array<block, 2>>  sender)
{

	for (u64 i = 0; i < choiceBits.size(); ++i)
	{

		u8 choice = choiceBits[i];
		const block & revcBlock = recv[i];
		//(i, choice, revcBlock);
		const block& senderBlock = sender[i][choice];

		//if (i%512==0) {
		//    std::cout << "[" << i << ",0]--" << sender[i][0] << std::endl;
		//    std::cout << "[" << i << ",1]--" << sender[i][1] << std::endl;
		//    std::cout << (int)choice << "-- " << recv[i] << std::endl;
		//}
		if (neq(revcBlock, senderBlock))
			std::cout << "eq(revcBlock, senderBlock"  << std::endl;

		if (eq(revcBlock, sender[i][1 ^ choice]))
			std::cout << "eq(revcBlock, sender[i][1 ^ choice])" << std::endl;

	}

}

void IknpOtExt_100Receive_Test_Impl()
{
	setThreadName("Sender");

	IOService ios(0);
	Session ep0(ios, "127.0.0.1", 1212, SessionMode::Server, "ep");
	Session ep1(ios, "127.0.0.1", 1212, SessionMode::Client, "ep");
	Channel senderChannel = ep1.addChannel("chl", "chl");
	Channel recvChannel = ep0.addChannel("chl", "chl");

	PRNG prng0(_mm_set_epi32(4253465, 3434565, 234435, 23987045));
	PRNG prng1(_mm_set_epi32(4253233465, 334565, 0, 235));

	u64 numOTs = 200;

	std::vector<block> recvMsg(numOTs), baseRecv(128);
	std::vector<std::array<block, 2>> sendMsg(numOTs), baseSend(128);
	BitVector choices(numOTs), baseChoice(128);
	choices.randomize(prng0);
	baseChoice.randomize(prng0);

	prng0.get((u8*)baseSend.data()->data(), sizeof(block) * 2 * baseSend.size());
	for (u64 i = 0; i < 128; ++i)
	{
		baseRecv[i] = baseSend[i][baseChoice[i]];
	}

	IknpOtExtSender sender;
	IknpOtExtReceiver recv;

	std::thread thrd = std::thread([&]() {



		recv.setBaseOts(baseSend);
		recv.receive(choices, recvMsg, prng0, recvChannel);
	});



	//{
	//    std::lock_guard<std::mutex> lock(Log::mMtx);
	//    for (u64 i = 0; i < baseOTs.receiver_outputs.size(); ++i)
	//    {
	//        std::cout << "i  " << baseOTs.receiver_outputs[i] << " " << (int)baseOTs.receiver_inputs[i] << std::endl;
	//    }
	//}
	sender.setBaseOts(baseRecv, baseChoice);
	sender.send(sendMsg, prng1, senderChannel);
	thrd.join();

	//for (u64 i = 0; i < baseOTs.receiver_outputs.size(); ++i)
	//{
	//    std::cout << sender.GetMessage(i, 0) << " " << sender.GetMessage(i, 1) << "\n" << recv.GetMessage(1) << "  " << recv.mChoiceBits[i] << std::endl;
	//}
	OT_100Receive_Test(choices, recvMsg, sendMsg);




	senderChannel.close();
	recvChannel.close();


	ep1.stop();
	ep0.stop();

	ios.stop();

	//senderNetMgr.Stop();
	//recvNetMg
}


int main(int argc, char** argv)
{
	
	AdaptiveMUL_test();
	IknpOtExt_100Receive_Test_Impl();

	Circuit_int_LessThan_Test();

    return 0;
}
