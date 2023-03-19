CFLAG = g++ -g
RELY = helperFun.hpp potato.h


target : ringmaster player
ringmaster : ringmaster.cpp $(RELY)
	$(CFLAG) $(RELY) ringmaster.cpp -o ringmaster
player : player.cpp $(RELY)
	$(CFLAG) $(RELY) player.cpp -o player

.PHONY: clean
clean:
	rm -rf player ringmaster