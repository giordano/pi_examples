#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "zdnn.h"

int main( int argc, char **argv ) {
   int i, num_steps = 10000;
   double step, x, sum, pi, taken;
   clock_t start, stop;  

   zdnn_tensor_desc pre_tfrmd_desc, tfrmd_desc;

   zdnn_ztensor range_t, x_t, sum_t, four_t, one_t, step_t;
   zdnn_status status;

   zdnn_data_types type = FP32;
   short element_size = 4; 

#ifdef STATIC_LIB
   zdnn_init();
#endif
 
   if (argc > 1) {
      num_steps = atol(argv[1]);
   }
   
   printf("Calculating PI using:\n"
          "  %ld slices\n"
          "  1 process\n", num_steps);
   
   start = clock();
   
   void *range_m = malloc(num_steps * element_size);
   void *x_m = malloc(num_steps * element_size);
   void *sum_m = malloc(num_steps * element_size);
   void *four_m = malloc(num_steps * element_size);
   void *one_m = malloc(num_steps * element_size);
   void *step_m = malloc(num_steps * element_size);

   sum = 0.0;
   step = 1.0 / num_steps;

   for (i=0;i<num_steps;i++) {
     ((float *)range_m)[i] = (float)(i + 0.5);
     ((float *)x_m)[i] = (float)(i + 0.5);
     ((float *)four_m)[i] = (float)(4.0);
     ((float *)one_m)[i] = (float)(1.0);
     ((float *)step_m)[i] = (float)(step);

   }

   zdnn_init_pre_transformed_desc(ZDNN_NHWC, type, &pre_tfrmd_desc, num_steps, 1, 1, 1);

   status = zdnn_generate_transformed_desc(&pre_tfrmd_desc, &tfrmd_desc);

   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &range_t);
   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &x_t);
   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &sum_t); 
   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &four_t);
   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &one_t);
   status = zdnn_init_ztensor_with_malloc(&pre_tfrmd_desc, &tfrmd_desc, &step_t); 

   status = zdnn_transform_ztensor(&range_t, range_m);
   status = zdnn_transform_ztensor(&x_t, x_m);
   status = zdnn_transform_ztensor(&one_t, one_m);
   status = zdnn_transform_ztensor(&four_t, four_m);
   status = zdnn_transform_ztensor(&step_t, step_m);

   status = zdnn_mul(&step_t, &range_t, &x_t);
   status = zdnn_mul(&x_t, &x_t, &x_t);

   status = zdnn_add(&one_t, &x_t, &range_t);
   status = zdnn_div(&four_t, &range_t, &sum_t);

   assert(status == ZDNN_OK);

   status = zdnn_transform_origtensor(&sum_t, sum_m);

   for (i=0;i<num_steps;i++){
     sum = sum + ((float *)sum_m)[i];
   }

   pi = sum * step;

   status = zdnn_free_ztensor_buffer(&range_t);
   status = zdnn_free_ztensor_buffer(&x_t);
   status = zdnn_free_ztensor_buffer(&sum_t);
   status = zdnn_free_ztensor_buffer(&one_t);
   status = zdnn_free_ztensor_buffer(&four_t);
   status = zdnn_free_ztensor_buffer(&step_t);

   free(range_m);
   free(x_m);
   free(sum_m);
   free(one_m);
   free(four_m);
   free(step_m);

   stop = clock();
   taken = ((double)(stop - start))/CLOCKS_PER_SEC;

   printf("Obtained value for PI: %.16g\n"
          "Time taken:            %.16g seconds\n", pi, taken);

   return 0;
}
