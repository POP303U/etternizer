#ifndef TOOLS_H

// Extracts a Double value from a yaml key
double ExtractDouble(const char *source, const char *key);

// Extracts a String value from a yaml key
char *ExtractValue(const char *source, const char *key);

#endif