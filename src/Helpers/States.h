#pragma once

struct EXECUTION_STATE {
  const static int FAILED = -1;
  const static int PENDING = 0;
  const static int SUCCESS = 1;
  const static int LOADING = 2;
};