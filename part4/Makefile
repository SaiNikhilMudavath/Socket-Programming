build:
	g++ client.cpp -o client
	g++ fifo.cpp -o fifo
	g++ fair_schedule.cpp -o fair_schedule
	g++ client_jfi.cpp -o jfi
	g++ fifo_jain.cpp -o fifo_jain
run-fifo:
	python3 runner.py fifo
run-rr:
	python3 runner.py rr
run:
	python3 runner.py all
plot:
	python3 plotter.py
fairness:
	@bash run_jfi.sh

