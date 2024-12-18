module: ./Source/T1S_RaspberryPiHardware.c ./Source/NCN26010.c ./Source/NCN26010_Main.c ./Source/T1S_TAPTCP-IP.c
	gcc -I./Include/ -Wall -o NCN26010_Driver -pthread ./Source/T1S_TAPTCP-IP.c ./Source/NCN26010.c ./Source/T1S_RaspberryPiHardware.c ./Source/NCN26010_Main.c -lpigpio -lrt

clean:
	$(RM) NCN26010_Driver 
