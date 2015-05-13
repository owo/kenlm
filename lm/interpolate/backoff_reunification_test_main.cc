#include "lm/interpolate/backoff_reunification.hh"

#include "lm/common/model_buffer.hh"
#include "lm/common/compare.hh"
#include "lm/common/ngram.hh"
#include "lm/word_index.hh"
#include "util/stream/io.hh"
#include "util/stream/sort.hh"

int main() {
  using namespace lm::interpolate;

  const std::string prob_file = "csorted-ngrams";
  const std::string boff_file = "backoffs";
  const std::string out_file = "interpolated";
  const std::size_t NUM_BLOCKS = 2;
  const std::size_t MAX_RAM = 1 << 30; // 1GB

  util::stream::SortConfig sort_config;
  sort_config.temp_prefix = "/tmp/";
  sort_config.buffer_size = 1 << 26;  // 64MB
  sort_config.total_memory = 1 << 30; // 1GB

  lm::common::ModelBuffer in_prob_buf(prob_file);
  lm::common::ModelBuffer in_boff_buf(boff_file);
  util::stream::Chains prob_chains(in_prob_buf.Order());
  util::stream::Chains backoff_chains(in_prob_buf.Order());

  for (std::size_t i = 0; i < in_prob_buf.Order(); ++i) {
    prob_chains.push_back(util::stream::ChainConfig(
        sizeof(lm::WordIndex) * (i + 1) + sizeof(float), NUM_BLOCKS, MAX_RAM));

    backoff_chains.push_back(
        util::stream::ChainConfig(sizeof(float), NUM_BLOCKS, MAX_RAM));
  }

  in_prob_buf.Source(prob_chains);
  in_boff_buf.Source(backoff_chains);

  util::stream::Chains output_chains(prob_chains.size());
  util::stream::Sorts<lm::SuffixOrder> sorts(prob_chains.size());
  for (size_t i = 0; i < prob_chains.size(); ++i) {
    output_chains.push_back(util::stream::ChainConfig(
        lm::NGram<lm::ProbBackoff>::TotalSize(i + 1), NUM_BLOCKS, MAX_RAM));

    sorts.push_back(prob_chains[i], sort_config,
                    lm::SuffixOrder(i + 1));
  }

  for (size_t i = 0; i < prob_chains.size(); ++i) {
    prob_chains[i].Wait();
    sorts[i].Output(prob_chains[i]);
  }

  util::stream::ChainPositions prob_pos(prob_chains);
  util::stream::ChainPositions boff_pos(backoff_chains);
  ReunifyBackoff(prob_pos, boff_pos, output_chains);

  lm::common::ModelBuffer output_buf(out_file, true, false);
  output_buf.Sink(output_chains);

  return 0;
}