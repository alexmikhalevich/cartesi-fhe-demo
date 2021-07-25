// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>
//#include <type_traits>

#include "fhe_data.h"
#include "string_cap_char_tfhe.h"
#include "string_cap_char_tfhe.types.h"

constexpr int kMainMinimumLambda = 120;

void FheStringCap(FheString& cipherresult, FheString& ciphertext, int data_size,
                  FheState& cipherstate,
                  const TFheGateBootstrappingCloudKeySet* bk) {
  for (int i = 0; i < data_size; i++) {
    my_package(cipherresult[i].get(), cipherstate.get(),
                            ciphertext[i].get(), bk);
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: string_cap_fhe_testbench string_input\n\n");
    return 1;
  }

  const char* input = argv[1];
  // generate a keyset
  TFHEParameters params(kMainMinimumLambda);

  // generate a random key.
  // Note: In real applications, a cryptographically secure seed needs to be
  // used.
  std::array<uint32_t, 3> seed = {314, 1592, 657};
  TFHESecretKeySet key(params, seed);

  std::string plaintext(input);
  size_t data_size = plaintext.size();
  std::cout << "plaintext(" << data_size << "):" << plaintext << std::endl;

  // Encrypt data
  auto ciphertext = FheString::Encrypt(plaintext, key);
  std::cout << "Encryption done" << std::endl;

  std::cout << "Initial state check by decryption: " << std::endl;
  std::cout << ciphertext.Decrypt(key) << "\n";
  std::cout << "\n";

  State st;
  FheState cipherstate(params.get());
  cipherstate.SetEncrypted(st, key.get());
  std::cout << "\t\t\t\t\tServer side computation:" << std::endl;
  // Perform string capitalization
  FheString cipher_result = {data_size, params};
  FheStringCap(cipher_result, ciphertext, data_size, cipherstate, key.cloud());
  std::cout << "\t\t\t\t\tComputation done" << std::endl;

  std::cout << "Decrypted result: ";
  std::cout << cipher_result.Decrypt(key) << "\n";
  std::cout << "Decryption done" << std::endl;
}
