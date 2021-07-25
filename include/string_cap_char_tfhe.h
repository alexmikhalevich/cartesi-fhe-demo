#ifndef STRING_CAP_CHAR_TFHE_H_
#define STRING_CAP_CHAR_TFHE_H_

#include "tfhe.h"
#include "tfhe_io.h"

int my_package(LweSample* result, LweSample* st, LweSample* c,
  const TFheGateBootstrappingCloudKeySet* bk);
#endif  // STRING_CAP_CHAR_TFHE_H_
