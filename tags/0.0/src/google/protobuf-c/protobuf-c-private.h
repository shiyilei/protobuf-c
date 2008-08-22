
/* === needs to be declared for the PROTOBUF_C_BUFFER_SIMPLE_INIT macro === */

void protobuf_c_buffer_simple_append (ProtobufCBuffer *buffer,
                                      size_t           len,
                                      const unsigned char *data);

/* === stuff which needs to be declared for use in the generated code === */

/* IntRange: helper structure for optimizing
     int => index lookups
   in the case where the keys are mostly consecutive values,
   as they presumably are for enums and fields.

   The data structures assumes that the values in the original
   array are sorted */
struct _ProtobufCIntRange
{
  int start_value;
  unsigned orig_index;
  /* NOTE: the number of values in the range can
     be inferred by looking at the next element's orig_index.
     a dummy element is added to make this simple */
};


/* === declared for exposition on ProtobufCIntRange === */
/* note: ranges must have an extra sentinel IntRange at the end whose
   orig_index is set to the number of actual values in the original array */
/* returns -1 if no orig_index found */
int protobuf_c_int_ranges_lookup (unsigned n_ranges,
                                  ProtobufCIntRange *ranges);

