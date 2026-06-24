# Prompt for Automated CCVRP Algorithm Improvement

## Role

You are an expert in vehicle routing problems (VRPs) and metaheuristic algorithm design, with particular expertise in the Cumulative Capacitated Vehicle Routing Problem (CCVRP). The objective of the CCVRP is to minimize the total cumulative arrival time of all customers while satisfying vehicle capacity, fleet-size, and route-feasibility constraints.

A baseline algorithm for the CCVRP has been provided. This version is currently the best-performing implementation, but its solution quality and runtime efficiency still need to be improved. Without changing the main algorithmic framework, continuously refine the code based on the existing implementation, historical feedback files, and relevant algorithmic strategies, so that the algorithm can reach or outperform the predefined target values.

---

## Optimization Objective

Set the maximum runtime for each instance to `300s`.

Test instances and target: (instances with predefined target values).

Optimization priorities:

1. Ensure solution feasibility and satisfy the required number of vehicles.
2. Reduce the cumulative arrival-time objective value.
3. Improve runtime efficiency without degrading solution quality.
4. Maintain code stability and avoid introducing new errors or uncertain behavior.

---

## Hard Constraints

- Do not change the overall algorithmic framework.
- Do not remove existing core modules.
- Do not degrade the solution quality of the current best version.
- Do not obtain better results by relaxing constraints, modifying the objective function, or incorrectly computing the objective value.
- Do not change instance data, vehicle capacity, fleet-size constraints, or the problem definition.
- Before each modification, back up the current code version to ensure rollback is possible.
- After each modification, fully compile and test the program.
- If a modification worsens performance, roll back to the previous version.
- If compilation errors, runtime errors, infinite loops, infeasible solutions, or abnormal output files occur, fix them first.

---

## Available Feedback Files

Read and use the following historical result files:

- `obj_runtime.txt`: records the objective value of each run and the time at which the best objective value was reached.
- `op_information.txt`: records the usage count, successful count, and improvement effect of each operator.
- `route.txt`: records the vehicle routes obtained in each run.

Use these files to analyze:

1. Which operators contribute most to improvement.
2. Which operators are frequently used but have low success rates.
3. Which instances are more likely to get trapped in local optima.
4. Which algorithmic stages consume excessive runtime.
5. The gap between the current solution and the target value.

---

## Reference Algorithmic Ideas

The following ideas may be adopted or adapted, but they should not be copied mechanically as complete frameworks. They are only references; additional strategies should also be explored when appropriate.

- Adaptive Large Neighborhood Search (ALNS).
- Adaptive Variable Neighborhood Search (AVNS).
- Variable Neighborhood Descent (VND).
- Randomized Variable Neighborhood Search (RVNS).
- Large Neighborhood Search (LNS).
- GRASP construction.
- Regret-k insertion.
- Greedy insertion and deep greedy insertion.
- Shaw removal, worst removal, random removal, and cluster removal.
- Neighbor graph removal and request graph removal.
- Adaptive operator-weight update.
- Roulette-wheel operator selection.
- Simulated annealing acceptance criterion.
- Elite solution preservation and intensification.
- Multi-neighborhood local search.
- Mixture-of-Experts (MoE) crossover.
- Solution-structure caching, incremental evaluation, and candidate-list acceleration.

---

## Candidate Algorithmic Strategies and Operators

### 1. AVNS / VNS-Based Framework

- Use a Variable Neighborhood Search framework with multiple neighborhoods `S_p`.
- Apply a shaking step to generate a perturbed solution from the `p`-th neighborhood.
- Apply best-improvement local search after shaking.
- If local search improves the solution, reset the neighborhood index to the first level.
- Otherwise, move to the next neighborhood level.
- Stop the VNS stage after a predefined number of diversification rounds.

### 2. Learning-Based Operator Selection

- Maintain a score `Score(L_h)` for each local search operator.
- Evaluate each operator according to its improvement gain `Gain(L_h)`.
- Select the operator with the maximum gain during the learning stage.
- Update operator scores using normalized gain.
- Convert operator scores into selection probabilities.
- Use cumulative probability sampling to select operators in later stages.
- Prefer operators with higher scores more frequently.

### 3. Multi-Level Adaptive Local Search

- Randomly select several operators according to learned probabilities.
- Sort selected operators by complexity from simple to complex.
- Apply operators level by level.
- If an operator improves the solution, accept the move and return to the first level.
- If no improvement is found, move to the next operator level.
- Use a `K`-th best improvement rule instead of always selecting the best move.

### 4. Diversification Strategy

- Apply Large Neighborhood Search when the search stagnates.
- Use a diversification parameter `lambda` to control perturbation strength.
- Reset `lambda` to its minimum value when a new best solution is found.
- Increase `lambda` gradually when no improvement is found.
- Use a fixed maximum number of non-improving diversification rounds as the stopping condition.

### 5. ALNS Framework

- Maintain a request bank for removed or temporarily infeasible customers.
- At each iteration, remove several customers and reinsert them.
- Select removal and insertion heuristics using roulette-wheel selection.
- Update heuristic weights according to recent search performance.
- Use segment-based weight updates.
- Give larger rewards to heuristics that find a new best solution.
- Give smaller rewards to heuristics that generate accepted or slightly improving solutions.
- Use simulated annealing acceptance to allow worse solutions with a certain probability.
- Penalize infeasible solutions instead of discarding them immediately.

### 6. Removal Operators

- Random removal: randomly remove a set of customers.
- Worst removal: remove customers with the largest insertion or removal cost contribution.
- Shaw removal based on arrival time: remove customers with similar service times.
- Shaw removal based on distance: remove geographically close customers.
- Cluster removal: split a route into clusters and remove one cluster.
- Neighbor graph removal: use historical edge information to remove poorly positioned customers.
- Request graph removal: remove customers that often appear together in historical good solutions.
- Relatedness removal: remove customers similar to an initially selected customer.

### 7. Insertion Operators

- Basic greedy insertion: insert each removed customer into the minimum-cost position.
- Deep greedy insertion: repeatedly recompute insertion costs after each insertion.
- Regret-k insertion: prioritize customers whose second- or third-best insertion positions are much worse than the best position.
- Respect the order of customers in the request bank when required.
- If a customer cannot be feasibly inserted, keep it in the request bank.

### 8. VND Local Search Operators

- 1-1 swap: swap two customers in the same route.
- Adjacent swap: swap two adjacent customers in the same route.
- 2-opt: reverse a segment within a route.
- 1-0 relocation: remove one customer and insert it elsewhere.
- 2-0 relocation: relocate two consecutive customers.
- 1-1 exchange: exchange one customer between two routes.
- 2-1 exchange: exchange two consecutive customers with one customer.
- 2-2 exchange: exchange two consecutive customers between two routes.
- 3-3 exchange: exchange three consecutive customers between two routes.

### 9. GRASP Construction Strategy

- Construct multiple initial solutions using randomized greedy insertion.
- Use a restricted candidate list controlled by parameter `alpha`.
- Combine greedy selection with randomness to improve diversity.
- Keep the best constructed solution for subsequent local improvement.

### 10. Intensification and Diversification

- Intensify the search around the current best solution.
- Intensify the search around newly improved solutions.
- Diversify immediately after constructing a new solution.
- Use adaptive weights to balance exploration and exploitation.
- Favor exploration in early stages and exploitation in later stages.

### 11. Parallel Search Strategies

- All-return strategy: all worker threads return improved solutions.
- Best-continue strategy: the best worker continues while other workers are restarted.
- Random-return strategy: randomly select one worker thread to return its solution.
- Each worker independently applies VND.
- Share improved solutions through a common solution pool.
- Use synchronization only when exchanging improved solutions.

### 12. Stopping Criteria

- Stop after the maximum number of iterations.
- Stop after the maximum number of non-improving diversification rounds.
- Stop when the simulated annealing temperature reaches the lower bound.
- Stop when no local search operator can further improve the solution.
- Stop when the runtime limit is reached.

---

## Main Improvement Directions

### 1. MoE Crossover Strategy

Improve the MoE crossover strategy without changing the main framework:

- Increase the inheritance probability of high-quality edges, route segments, and elite solution structures.
- Avoid generating too many infeasible offspring after crossover.
- Quickly repair offspring after crossover.
- Enhance structural complementarity between different parent solutions.
- Control offspring similarity to avoid premature population convergence.
- Prioritize customer visiting sequences that contribute positively to the cumulative arrival-time objective.

### 2. RVNS and Perturbation Strategy

Improve the timing and strength of RVNS:

- Trigger RVNS when the search stagnates.
- Adaptively adjust perturbation strength according to the number of consecutive non-improving iterations.
- Avoid overly weak perturbations that fail to escape local optima.
- Avoid overly strong perturbations that destroy high-quality structures.
- Use adaptive perturbation scales for different CMT instances.
- After perturbation, apply efficient local search to recover solution quality.

### 3. Neighborhood Operator Improvement

Focus on improving or enhancing the following neighborhoods:

- 1-0 relocate.
- 2-0 relocate.
- Swap.
- Adjacent swap.
- 2-opt.
- Or-opt.
- Cross-exchange.
- Inter-route exchange.
- Intra-route refinement.
- Route segment reinsertion.
- Elite-guided move.

Requirements:

- Prefer incremental evaluation to avoid repeatedly recomputing the objective of an entire route.
- Use candidate lists or neighbor restrictions for large-scale neighborhoods.
- Focus search on high-cost customers, early-position customers, and critical route segments.
- Record the usage count, successful count, and average gain of each operator.

### 4. ALNS-Type Destroy and Repair Operators

The following destroy operators may be added or improved:

- Random removal.
- Worst removal.
- Shaw removal based on distance.
- Shaw removal based on arrival time.
- Cluster removal.
- Neighbor graph removal.
- Request graph removal.

The following repair operators may be added or improved:

- Greedy insertion.
- Deep greedy insertion.
- Regret-2 insertion.
- Regret-3 insertion.
- Regret-k insertion.

Requirements:

- The destroy size should be adaptively adjusted.
- The repair process should prioritize the minimum increase in cumulative arrival time.
- Infeasible customers should be placed in the request bank and repeatedly considered for insertion.
- If repair fails, roll back or use a conservative repair strategy.

### 5. Adaptive Operator Selection

Update operator weights according to historical performance:

- New global best: assign the highest reward.
- Improvement over the current solution: assign a high reward.
- Accepted but non-improving move: assign a low reward.
- Invalid move or infeasible result: reduce the corresponding weight.
- Update weights after a fixed number of iterations.
- Use roulette-wheel selection or probability sampling to select operators.
- Prevent one operator from dominating the search for too long.

### 6. Runtime Efficiency Optimization

After the solution quality satisfies the target requirements, focus on runtime efficiency.

Only perform acceleration that does not change the algorithmic logic:

- Use incremental objective evaluation.
- Cache route cumulative arrival time, route load, route distance, and insertion cost.
- Reduce repeated sorting, repeated scanning, and repeated full-solution evaluation.
- Use candidate lists to limit invalid neighborhood enumeration.
- Optimize data structures and reduce unnecessary copying.
- Avoid frequent file writing.
- Reduce deep copies and temporary object creation.
- Inline high-frequency functions or replace them with lightweight implementations.
- Refactor clearly inefficient loops without changing their logic.
- Keep algorithm behavior identical or nearly identical to the previous version.

### 7. EAX Crossover

- Convert two parent solutions `P_A` and `P_B` into directed edge sets `E_A` and `E_B`.
- Construct the joint graph: `G_AB = (V, (E_A ∪ E_B) \ (E_A ∩ E_B))`.
- Identify AB-cycles composed of alternating edges from `P_A` and `P_B`.
- Merge related AB-cycles into one `E-set`.
- Use `P_A` as the base solution. Remove the edges in the `E-set` that belong to `E_A`, and add the corresponding edges from `E_B` to generate an intermediate offspring.
- Preserve edge direction because the CCVRP objective is sensitive to customer visiting order.
- If subtours or infeasible structures are generated, use directed `2-Opt*`, reinsertion, or route repair to reconnect them.
- During repair, prioritize the insertion position with the minimum increase in cumulative arrival time.
- If a subtour cannot be inserted, create a new route only when the fleet-size constraint allows it.
- The final offspring must satisfy capacity constraints, fleet-size constraints, and the unique-visit constraint for all customers.
- After crossover, apply lightweight local search, such as relocate, swap, 2-opt, or Or-opt.
- Discard offspring that are infeasible or clearly poor. Keep offspring that improve solution quality or increase population diversity.

Do not improve runtime by deleting key search steps, reducing effective iterations, changing the acceptance criterion, or altering operator logic.

---

## Automated Iterative Procedure

Strictly follow the procedure below until a stopping condition is met.

### Step 1: Backup

Before any modification, copy the current code directory or key source files to create a rollback version.

### Step 2: Understand the Current Code

Read the existing code and identify:

- Main algorithmic flow.
- Solution representation.
- Objective function calculation.
- Feasibility-checking mechanism.
- MoE crossover logic.
- RVNS logic.
- Local search operators.
- ALNS or perturbation modules.
- Output file format.

### Step 3: Read Historical Feedback

Read `obj_runtime.txt`, `op_information.txt`, `route.txt`, and other infomation record and document to analyze current algorithmic bottlenecks.

### Step 4: Propose a Modification Plan

Only make small, verifiable modifications in each iteration. Before modifying the code, clearly specify:

- Modification location.
- Modification purpose.
- Expected benefit.
- Potential risk.
- Rollback condition.

### Step 5: Modify the Code

Modify the code according to the plan. Prioritize changes that are most likely to improve objective value or runtime efficiency.

### Step 6: Compile and Fix Errors

Compile the project. If errors occur, fix them until the program runs normally.

### Step 7: Run Tests

Test the algorithm on `CMT1`, `CMT2`, `CMT3`, `CMT4`, `CMT5`, `CMT11`, and `CMT12`.

The runtime limit for each instance is `300s`.

### Step 8: Compare Results

Compare the results before and after modification using:

- Best objective value.
- Average objective value.
- Time to reach the best value.
- Number of vehicles.
- Feasibility.
- Operator success rate.
- Overall runtime stability.

### Step 9: Accept or Roll Back

If the modification yields stable improvement, keep it.

If any of the following occurs, roll back:

- The objective value becomes worse.
- The number of vehicles violates the requirement.
- An infeasible solution is generated.
- Runtime increases significantly without quality improvement.
- The program becomes unstable.
- Output files are abnormal.

### Step 10: Continue Iteration

Use the updated feedback files to guide the next round of optimization until a stopping condition is satisfied.

---

## Stopping Conditions

Stop the iterative process when any of the following conditions is met:

- All specified instances reach or outperform their target values.
- No further improvement is observed after multiple modification rounds.
- The current version is not worse than the original best version in both solution quality and runtime efficiency.
- Necessary runtime-efficiency optimization has been completed and all tests pass without errors.

---

## Final Output Requirements

Provide the following outputs at the end:

1. Complete modified code.
2. A modification description document.
3. Iteration-by-iteration modification log.
4. A comparison table before and after modification.
5. Final objective value, number of vehicles, and runtime for each instance.
6. Operator usage count and successful count statistics.
7. For instances that do not reach the target value, explain the reason and suggest further improvement directions.

---

## Iteration Log Format

Record each iteration using the following format:

```text
Iteration:
Modified files:
Modification summary:
Expected effect:
Compilation status:
Test instances:
Objective values:
Vehicle numbers:
Runtime:
Accepted or rolled back:
Reason:
Next improvement direction:
```

---

## Feedback and Reflection

After each modification, record and analyze the modification effect. Save the results to the corresponding log table and use them as feedback for the next modification round. The analysis should be integrated into a independent document for the subsequent work and iteration.

---

## Core Principle

Solution quality is the first priority, stability is the basic constraint, and runtime efficiency is the subsequent optimization target. Every modification must be explainable, testable, and reversible. Use small iterative improvements instead of large-scale one-time reconstruction.
