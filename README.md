# Student

- Name: Shivu Donmardi Gorva
- NUID: 002599393

# Assignment

- Course: CS 7610
- Assignment #: 2

# Chandi-Lamport snapshot algorithm state diagram for a process

- Q: {q<sub>0</sub>, q<sub>1</sub>, q<sub>2</sub>} 
- Σ: {C, C<sub>x</sub>, m<sub>i</sub>}
- Start state: q<sub>0</sub>
- Final state: q<sub>2</sub>

## Symbols (Σ)

- C: Set of open channels.
- C<sub>x</sub>: Set of closed channels.
- m<sub>i</sub>: Received marker from process i.

## States (Q)

### q<sub>0</sub>
- Initial state.
- Process hasn't received any marker.
- All channels open.

### q<sub>1</sub>
- Recording channels c ∈ (C - C<sub>x</sub>)

### q<sub>2</sub>
- Done

## Diagram




# How to run

- The application can be run exactly as specified by the assignment specification.
- All test cases are working.

Note: I wasn't able to figure out how to implement circular queue in C. Since read channels use these limited sized queues, the app can raise an exception if the queue is exhausted. Thus we can only run the application upto ~900 states which is quite sufficient for the provided testcases. If required the queue size can be increased by increasing the value of the `channelSize` configuration paramter. This is definitely something we wouldn't have in a production application, if I find more bandwidth, will update the data structure to a circular queue in the future.

# Configuration Parameters

- All configuration parameters can be found in `config/config.c`
