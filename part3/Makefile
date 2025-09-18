build:
	g++ server_aloha.cpp -o server_aloha
	g++ server_sense.cpp -o server_sense
	g++ client_slotted_aloha.cpp -o client_slotted_aloha
	g++ client_beb.cpp -o client_beb
	g++ client_sense.cpp -o client_sense
run-aloha:
	python3 runner.py aloha
run-beb:
	python3 runner.py beb
run-cscd:
	python3 runner.py sense
run:
	python3 runner.py all
plot:
	python3 plotter.py

