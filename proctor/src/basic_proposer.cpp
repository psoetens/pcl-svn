#include "proctor/basic_proposer.h"

namespace pcl
{
  
  namespace proctor
  {

    void
    BasicProposer::getProposed(int max_num, Entry &query, std::vector<std::string> &input, std::vector<std::string> &output)
    {
      std::vector<std::string>::iterator database_it;

      vector<Candidate> ballot;
      for ( database_it = input.begin() ; database_it != input.end(); database_it++ ) {
        std::string target_id = (*database_it);
        Entry target = (*database_)[target_id];

        Candidate* candidate = new Candidate;
        candidate->id = target_id;
        candidate->votes = getVotes(query, target);
        ballot.push_back(*candidate);
      }

      selectBestCandidates(max_num, ballot, output);
    }

    double
    BasicProposer::getVotes(Entry &query, Entry &match)
    {
      int max_votes = 1; // TODO Set this to what?

      double votes = 0;
      for (unsigned int pi = 0; pi < query.indices->size(); pi++) {
        vector<int> indices;
        vector<float> distances;

        StopWatch s;
        int num_found = match.tree->nearestKSearch(*query.features, pi, max_votes, indices, distances);

        for (int ri = 0; ri < num_found; ri++) {
          votes += 1. / (distances[ri] + numeric_limits<float>::epsilon());
          //votes -= distances[ri];
        }
      }

      return votes;
    }

  }

}
