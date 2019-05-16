// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#ifndef RUN_CROWN_CONCOLIC_SEARCH_H__
#define RUN_CROWN_CONCOLIC_SEARCH_H__

#include <map>
#include <vector>
#include <ext/hash_map>
#include <ext/hash_set>
#include <time.h>

/*
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
*/

#include "base/basic_types.h"
#include "run_crown/symbolic_execution.h"

using std::map;
using std::vector;
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;

namespace crown {

class Search {
 public:
	 Search(const string& program, int max_iterations);
	 virtual ~Search();

	 virtual void Run() = 0;

 protected:
	 vector<branch_id_t> branches_;
	 vector<branch_id_t> paired_branch_;
	 vector<function_id_t> branch_function_;
	 vector<bool> covered_;
	 vector<bool> total_covered_;
	 branch_id_t max_branch_;
	 unsigned int num_covered_;
	 unsigned int total_num_covered_;

	 vector<bool> reached_;
	 vector<unsigned int> branch_count_;
	 function_id_t max_function_;
	 unsigned int reachable_functions_;
	 unsigned int reachable_branches_;
	 unsigned int sym_path_length;
	 unsigned int con_path_length;

	 void send_Msg(char* message);
	 void send_File(char* file_name);
	 int exec_cil_in_rasp();

	 time_t start_time_;

	 typedef vector<branch_id_t>::const_iterator BranchIt;

	 bool SolveAtBranch(SymbolicExecution& ex,
			 size_t branch_idx,
			 vector<Value_t>* input);

	 bool CheckPrediction(const SymbolicExecution& old_ex,
			 const SymbolicExecution& new_ex,
			 size_t branch_idx);

	 void RunProgram(const vector<Value_t>& inputs, SymbolicExecution* ex);
	 bool UpdateCoverage(const SymbolicExecution& ex);
	 bool UpdateCoverage(const SymbolicExecution& ex,
			 set<branch_id_t>* new_branches);

	 void RandomInput(const map<var_t,type_t>& vars, vector<Value_t>* input);

 private:
	 const string program_;
	 const int max_iters_;
	 int num_iters_;

	 /*
		struct sockaddr_un sock_;
		int sockd_;
	  */

	 void WriteInputToFileOrDie(const string& file, const vector<Value_t>& input, const vector<unsigned char> & h, const vector<unsigned char> & l,
					 												const vector<unsigned char> & idx);
	 void WriteCoverageToFileOrDie(const string& file);
	 int LaunchProgram(const vector<Value_t>& inputs);
	 int LaunchProgram();
};


class BoundedDepthFirstSearch : public Search {
public:
	explicit BoundedDepthFirstSearch(const string& program,
			int max_iterations,
			int max_depth,
			bool reverse);
	virtual ~BoundedDepthFirstSearch();

	virtual void Run();

private:
	int max_depth_;
	bool reverse_;
	//reverse set to 1 for reverse DFS
	//reverse set to 0 for DFS

	void reverse_DFS(size_t pos, int depth, SymbolicExecution& prev_ex);
	void DFS(size_t pos, int depth, SymbolicExecution& prev_ex);
};


/*
   class OldDepthFirstSearch : public Search {
   public:
   explicit OldDepthFirstSearch(const string& program,
   int max_iterations);
   virtual ~OldDepthFirstSearch();

   virtual void Run();

   private:

   void DFS(size_t pos, SymbolicExecution& prev_ex);
   };
 */

class RandomInputSearch : public Search {
public:
	RandomInputSearch(const string& program, int max_iterations);
	virtual ~RandomInputSearch();

	virtual void Run();

private:
	SymbolicExecution ex_;
};


class RandomSearch : public Search {
public:
	RandomSearch(const string& program, int max_iterations);
	virtual ~RandomSearch();

	virtual void Run();

private:
	SymbolicExecution ex_;

	void SolveUncoveredBranches(size_t i, int depth,
			SymbolicExecution& prev_ex);

	bool SolveRandomBranch(vector<Value_t>* next_input, size_t* idx);
};


class UniformRandomSearch : public Search {
public:
	UniformRandomSearch(const string& program, int max_iterations, size_t max_depth);
	virtual ~UniformRandomSearch();

	virtual void Run();

private:
	SymbolicExecution prev_ex_;
	SymbolicExecution cur_ex_;

	size_t max_depth_;

	void DoUniformRandomPath();
};


// Search looks like:
//  (1) Do random inputs for some amount of time.
//  (2) Do a local search repeatedly in some area, then continue the random search.
class HybridSearch : public Search {
	public:
		HybridSearch(const string& program, int max_iterations, int step_size);
		virtual ~HybridSearch();

		virtual void Run();

	private:
		void RandomLocalSearch(SymbolicExecution* ex, size_t start, size_t end);
		bool RandomStep(SymbolicExecution* ex, size_t start, size_t end);

		int step_size_;
};


class CfgBaselineSearch : public Search {
	public:
		CfgBaselineSearch(const string& program, int max_iterations);
		virtual ~CfgBaselineSearch();

		virtual void Run();

	private:
		SymbolicExecution success_ex_;

		bool DoSearch(int depth, int iters, int pos, SymbolicExecution& prev_ex);
};


class CfgHeuristicSearch : public Search {
public:
	CfgHeuristicSearch(const string& program, int max_iterations);
	virtual ~CfgHeuristicSearch();

	virtual void Run();

private:
	typedef vector<branch_id_t> nbhr_list_t;
	vector<nbhr_list_t> cfg_;
	vector<nbhr_list_t> cfg_rev_;
	vector<size_t> dist_;

	static const size_t kInfiniteDistance = 10000;

	int iters_left_;

	SymbolicExecution success_ex_;

	// Stats.
	unsigned num_inner_solves_;
	unsigned num_inner_successes_pred_fail_;
	unsigned num_inner_lucky_successes_;
	unsigned num_inner_zero_successes_;
	unsigned num_inner_nonzero_successes_;
	unsigned num_inner_recursive_successes_;
	unsigned num_inner_unsats_;
	unsigned num_inner_pred_fails_;

	unsigned num_top_solves_;
	unsigned num_top_solve_successes_;

	unsigned num_solves_;
	unsigned num_solve_successes_;
	unsigned num_solve_sat_attempts_;
	unsigned num_solve_unsats_;
	unsigned num_solve_recurses_;
	unsigned num_solve_pred_fails_;
	unsigned num_solve_all_concrete_;
	unsigned num_solve_no_paths_;

	void UpdateBranchDistances();
	void PrintStats();
	bool DoSearch(int depth, int iters, int pos, int maxDist, SymbolicExecution& prev_ex);
	bool DoBoundedBFS(int i, int depth, SymbolicExecution& prev_ex);
	void SkipUntilReturn(const vector<branch_id_t> path, size_t* pos);

	bool FindAlongCfg(size_t i, unsigned int dist,
			const SymbolicExecution& ex,
			const set<branch_id_t>& bs);

	bool SolveAlongCfg(size_t i, unsigned int max_dist,
			SymbolicExecution& prev_ex);

	void CollectNextBranches(const vector<branch_id_t>& path,
			size_t* pos, vector<size_t>* idxs);

	size_t MinCflDistance(size_t i,
			const SymbolicExecution& ex,
			const set<branch_id_t>& bs);
};

}  // namespace crown

#endif  // RUN_CROWN_CONCOLIC_SEARCH_H__

