#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include"hw3_solution.h"

//-----------------Element Queue Functions------------------------------------

// The following function initializes all "D" (i.e., total_departure) elements in the queue
// 1. It uses the seed value to initialize random number generator
// 2. Generates "D" exponentially distributed inter-arrival times based on lambda
//    And inserts "D" elements in queue with the correct arrival times
//    Arrival time for element i is computed based on the arrival time of element i-1 added to element i's generated inter-arrival time
//    Arrival time for first element is just that element's generated inter-arrival time
// 3. Generates "D" exponentially distributed service times based on mu
//    And updates each queue element's service time in order based on generated service times
// 4. Returns a pointer to the generated queue
void ElementQueue::InitializeQueue(int seed, double lambda, double mu, int total_departures){
	double u;
	double x;
	double random_num;
	//double inter_arrival_times[total_departures];
	//double arrival_times[total_departures];
	double arrival_time;
	double service_time;
	//ElementQueueNode curr;
	srand(seed);
	
	for(int i = 0; i < total_departures; i++){
		do { u = rand() / (double)RAND_MAX; } while (u == 0.0);
		x = (-1.0/lambda) * log(u);			//inter-arrival time
		
		if(i == 0){
			arrival_time = x;
		} else {
			arrival_time += x;
		}

		
		ElementArray[i].arrival_time = arrival_time;
	}

	for(int j = 0; j < total_departures; j++){
		do { random_num = rand() / (double)RAND_MAX; } while (random_num == 0.0);
		service_time = (-1.0/mu) * log(random_num);

		ElementArray[j].service_time = service_time;
	}
	
}

// Create and add an event to the event queue and insert it in the correct priority order based on event time
void EventQueue::ScheduleEvent(double etime, int etype, ElementQueueNode* qnode) {
	EventQueueNode* new_node = CreateEvent(etime, etype, qnode);
	num_events++;
	EventQueueNode* current;
	EventQueueNode* prev = NULL;

	if(head == NULL){
		head = new_node;
		new_node->next = NULL;
	} else {
		current = head;
		while(current != NULL){
			if(etime < current->event_time){
				if(prev == NULL){
					head = new_node;
					new_node->next = current;
					return;
				}
				
				prev->next = new_node;
				new_node->next = current;
				return;
			} else if (etime == current->event_time){
				if(etype == 1){			// if new_node's etype is arrival
					if(current->event_type == 3){		// 3 is departure
						new_node->next = current->next;
						current->next = new_node;
						return;
					} else if (current->event_type == 2){	//2 is start service
						new_node->next = current->next;
						current->next = new_node;
						return;
					}
				} else if (current->event_type == 1){		//if current's event_type is arrival
					if(etype == 3 || etype == 2){
						if(prev == NULL){
							head = new_node;
							new_node->next = current;
							return;
						}

						prev->next = new_node;
						new_node->next = current;
						return;
					}
				} else if (etype == 2){
					if(current->event_type == 3){
						new_node->next = current->next;
						current->next = new_node;
						return;
					}
				} else if (etype == 3){
					if(current->event_type == 2){
						if(prev == NULL){
							head = new_node;
							new_node->next = current;
						} else {
							prev->next = new_node;
							new_node->next = current;
						}

						return;
					}
				}
			}

			prev = current;
			current = current->next;
		}

		new_node->next = NULL;
		prev->next = new_node;
	}
}

// Remove the node at the head of the event queue from the queue and return it to caller
// AFter this function is called, you should call RemoveEvent() provided in the header file to free space
EventQueueNode* EventQueue::GetNextEvent() {
	EventQueueNode* node = head;
	if(head != nullptr){
		head = head->next;
		num_events--;
	}
	
	return node;
}

// Use the M/M/1 formulas from class to compute E(n), E(r), E(w), p0
// The computed values should be stored in computed_stats[0]-[3] to be printed
void Simulation::GenerateComputedStatistics(){
	double rho = lambda / mu;

	double E_of_n = rho / (1 - rho);

	double E_of_r = 1 / (mu * (1 - rho));

	double E_of_w = rho / (mu * (1 - rho));

	double p_zero = 1 - rho;

	computed_stats[0] = E_of_n;
	computed_stats[1] = E_of_r;
	computed_stats[2] = E_of_w;
	computed_stats[3] = p_zero;
}

// This function is called from simulator if the next event is an arrival
// Should update simulated statistics based on new arrival
// Should update system state
// Should schedule the arrival event for the next element in the Element Queue (except for last arrival)
// Should schedule a start service event if the server is idle
// *arriving_node points to queue node that arrived
void Simulation::ProcessArrival(ElementQueueNode* arriving_node){
		//simulated statistics
	//arrival_count++;
	
	//current_time = arriving_node->arrival_time;		//system state
	n += 1;
	//cumulative_number++;

	ElementQueueNode* next_element = ElementQ->AdvanceToNextElement();
	if(next_element != nullptr){
		EventQ->ScheduleEvent(next_element->arrival_time, 1, next_element);
	}

	if(server_busy == 0){
		EventQ->ScheduleEvent(current_time, 2, arriving_node);
	}
}

// This function is called from simulator if next event is "start_service"
// Should update queue statistics and system state
// Should schedule a departure event based on *served_node's service time
void Simulation::StartService(ElementQueueNode* served_node){
	
	server_busy = 1; 	//system state

	cumulative_waiting = cumulative_waiting + (current_time - served_node->arrival_time);	//queue statistics

	//EventQ->RemoveEvent(EventQ->GetNextEvent());	//?

	double departure_time = current_time + served_node->service_time;
	EventQ->ScheduleEvent(departure_time, 3, served_node);

	next_departure_time = departure_time;
	//cumulative_idle_times += (current_time - next_departure_time);
}

// This function is called from simulator if the next event is a departure
// Should update simulated queue statistics and update system state
// Should schedule a start service event for the next element in ElementQ after *departing_node if it's waiting
void Simulation::ProcessDeparture(ElementQueueNode* departing_node){
	//server_busy = 0;	//system state
	n--;

	departure_count++;		//queue statistics
	cumulative_response = cumulative_response + (current_time - departing_node->arrival_time);

	//ElementQueueNode* current_element = ElementQ->GetCurrentElement();
	if(n > 0){		//ask prof about this
		ElementQueueNode* next_element = ElementQ->GetElementAtIndex(departure_count);
		EventQ->ScheduleEvent(current_time, 2, next_element);
	} else{
		server_busy = 0;
	}
}

// This is the main simulator function
// Should run until departure_count == total_departures
// Needs to schedule the first arrival on the Event Queue
// Each iteration determines what the next event is from the Event Queue
// Calls appropriate function based on next event: ProcessArrival(), StartService(), ProcessDeparture()
// Advances current_time to next event
// Updates queue statistics if needed
// Print statistics if departure_count is a multiple of print_period
// Don't print end of simulation statistics which will be printed from main()
void Simulation::RunSimulation(){
	ElementQueueNode* first_node = ElementQ->GetCurrentElement();
	//ElementQ->AdvanceToNextElement();
	
	EventQ->ScheduleEvent(first_node->arrival_time, 1, first_node);
	
	EventQueueNode* event;
	while(departure_count < total_departures){
		event = EventQ->GetNextEvent();
		
		current_time = event->event_time;
		cumulative_number += (current_time - prev_time) * n;
		cumulative_idle_times += (server_busy == 0 ? 1 : 0) * (current_time - prev_time);
		prev_time = current_time;

		if(event->event_type == 1){
			ProcessArrival(event->qnode);
		} 
		else if(event->event_type == 2){
			StartService(event->qnode);
		}
		else if(event->event_type == 3){
			ProcessDeparture(event->qnode);
			if(departure_count % print_period == 0){
				PrintStatistics(departure_count, print_period, lambda);
				printf("\n");
			}
		}

		EventQ->RemoveEvent(event);

		simulated_stats[0] = cumulative_number / current_time;
		simulated_stats[1] = cumulative_response / departure_count;
		simulated_stats[2] = cumulative_waiting / departure_count;
		simulated_stats[3] = cumulative_idle_times / current_time;
	}
}


// Program's main function
int main(int argc, char* argv[]){

	// input arguments lambda, mu, D1, D, S
	if(argc >= 6){

		double lambda = atof(argv[1]);
		double mu = atof(argv[2]);
		int print_period = atoi(argv[3]);
		int total_departures = atoi(argv[4]);
		int random_seed = atoi(argv[5]);
   
	   	// Add error checks for input variables here, exit with exit code 1 if input is incorrect

   		// If no input errors, generate M/M/1 computed statistics based on formulas from class
   		Simulation* s = new Simulation(random_seed, lambda, mu, print_period, total_departures);

   		// Start Simulation
		printf("Simulating M/M/1 queue with lambda = %f, mu = %f, D1 = %d, D = %d, S = %d\n", lambda, mu, print_period, total_departures, random_seed); 
		s->RunSimulation();
		s->PrintStatistics(total_departures, print_period, lambda);
		delete s;
		return 0;
	}
	else {
		printf("Insufficient number of arguments provided!\n");
		return 1;
	}
}
