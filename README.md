# Repository File Description

This repository contains the source code, benchmark instances, vehicle-number settings, experimental results, intermediate code files, and the prompt document used for solving the Cumulative Capacitated Vehicle Routing Problem (CCVRP).

## Files

- `CCVRP.cpp`  
  The main C++ source file, including the `main` function and the implementation of the proposed algorithm.

- `Original_CCVRP.cpp`  
  The original algorithm framework provided to the LLM as the initial code for iterative improvement.

- `information_print.zip`  
  Contains the vehicle routes of the final solutions generated during the training and running processes for each benchmark instance.

- `CCVRP_experiment_result.xlsx`  
  Contains the final experimental results, including the results of ten independent runs of the algorithm on each benchmark instance.

- `CCVRP_temcode.zip`  
  Contains intermediate algorithms and code versions generated during the LLM-based iterative development process but not adopted in the final implementation.

- `Instance.zip`  
  Contains all benchmark instances used for testing.

- `Instance Vehicle Num.zip`  
  Contains the corresponding number of available vehicles for each benchmark instance.

- `prompt.md`  
  The integrated and optimized prompt document used in the LLM-based algorithm design process.

## Running Notes

When running `CCVRP.cpp`, the benchmark instances in `Instance.zip` and the corresponding vehicle-number settings in `Instance Vehicle Num.zip` are used as input. The input format and file paths are specified in the C++ code.

Before running the code on a local machine, please update the input file paths according to your own directory structure. The output path should also be modified as needed based on your local configuration.
