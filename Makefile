build : client_stub_lin_v1-4.cpp
	g++ -Wall -o2 -o tabla client_stub_lin_v1-4.cpp def.cpp

run : tabla
	for i in 1 2 3; do ./tabla $(IP) $(PORT) 8 8; done

clean : tabla
	rm tabla
