#include "sample.h"
#include "dump-core.h"

#include "box3d/box3d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#define CHECKPOINT_INTERVAL 50
#define MAX_FRAMES 300

struct Options
{
  const char* sampleName = NULL;
  const char* outputPath = NULL;
  int checkpointInterval = CHECKPOINT_INTERVAL;
  int maxFrame = MAX_FRAMES;
  int startFrame = 0;
  bool listJson = false;
  bool help = false;
  bool maxFrameExplicit = false;
  std::vector<int> exactFrames;
};

static int find_sample_index(const char* name)
{
  for (int i = 0; i < g_sampleCount; i++)
  {
    if (strcmp(g_sampleEntries[i].Name, name) == 0)
      return i;
  }
  return -1;
}

static void print_usage(FILE* out)
{
  fprintf(out, "Usage: reference-dump [options] <SampleName> [output.json]\n");
  fprintf(out, "Options:\n");
  fprintf(out, "  --help                         Show this help\n");
  fprintf(out, "  --list-json                    Print registered samples as JSON\n");
  fprintf(out, "  --checkpoint-interval <frames> Emit periodic checkpoints (default: 50)\n");
  fprintf(out, "  --max-frames <frame>           Last frame to simulate (default: 300)\n");
  fprintf(out, "  --start-frame <frame>          First frame eligible for dumping (default: 0)\n");
  fprintf(out, "  --frames <a,b,c>               Dump exact frames instead of periodic checkpoints\n");
}

static void print_samples_text(FILE* out)
{
  fprintf(out, "Available samples:\n");
  for (int i = 0; i < g_sampleCount; i++)
  {
    fprintf(out, "  %s\n", g_sampleEntries[i].Name);
  }
}

static void print_json_string(FILE* out, const char* value)
{
  fputc('"', out);
  for (const char* c = value; *c != '\0'; ++c)
  {
    if (*c == '"' || *c == '\\')
    {
      fputc('\\', out);
    }
    fputc(*c, out);
  }
  fputc('"', out);
}

static void print_samples_json(FILE* out)
{
  fprintf(out, "{\"samples\":[");
  for (int i = 0; i < g_sampleCount; i++)
  {
    fprintf(out, "%s{\"category\":", i == 0 ? "" : ",");
    print_json_string(out, g_sampleEntries[i].Category);
    fprintf(out, ",\"name\":");
    print_json_string(out, g_sampleEntries[i].Name);
    fprintf(out, "}");
  }
  fprintf(out, "]}\n");
}

static bool parse_int(const char* text, int* out)
{
  char* end = NULL;
  long value = strtol(text, &end, 10);
  if (end == text || *end != '\0' || value < 0 || value > 2147483647L)
  {
    return false;
  }
  *out = (int)value;
  return true;
}

static bool parse_frame_list(const char* text, std::vector<int>* out)
{
  const char* cursor = text;
  while (*cursor != '\0')
  {
    char* end = NULL;
    long value = strtol(cursor, &end, 10);
    if (end == cursor || value < 0 || value > 2147483647L)
    {
      return false;
    }
    out->push_back((int)value);
    if (*end == '\0')
    {
      return true;
    }
    if (*end != ',')
    {
      return false;
    }
    cursor = end + 1;
  }
  return !out->empty();
}

static bool should_dump_frame(const Options& options, int frame)
{
  if (frame < options.startFrame)
  {
    return false;
  }
  if (!options.exactFrames.empty())
  {
    for (int exactFrame : options.exactFrames)
    {
      if (frame == exactFrame)
      {
        return true;
      }
    }
    return false;
  }
  return options.checkpointInterval > 0 && frame % options.checkpointInterval == 0;
}

static bool parse_options(int argc, char* argv[], Options* options)
{
  for (int i = 1; i < argc; i++)
  {
    const char* arg = argv[i];
    if (strcmp(arg, "--help") == 0)
    {
      options->help = true;
    }
    else if (strcmp(arg, "--list-json") == 0)
    {
      options->listJson = true;
    }
    else if (strcmp(arg, "--checkpoint-interval") == 0 || strcmp(arg, "--max-frames") == 0 || strcmp(arg, "--start-frame") == 0 || strcmp(arg, "--frames") == 0)
    {
      if (i + 1 >= argc)
      {
        fprintf(stderr, "Missing value for %s\n", arg);
        return false;
      }
      const char* value = argv[++i];
      if (strcmp(arg, "--frames") == 0)
      {
        if (!parse_frame_list(value, &options->exactFrames))
        {
          fprintf(stderr, "Invalid frame list: %s\n", value);
          return false;
        }
      }
      else
      {
        int parsed = 0;
        if (!parse_int(value, &parsed))
        {
          fprintf(stderr, "Invalid integer for %s: %s\n", arg, value);
          return false;
        }
        if (strcmp(arg, "--checkpoint-interval") == 0)
        {
          if (parsed <= 0)
          {
            fprintf(stderr, "--checkpoint-interval must be greater than zero\n");
            return false;
          }
          options->checkpointInterval = parsed;
        }
        else if (strcmp(arg, "--max-frames") == 0)
        {
          options->maxFrame = parsed;
          options->maxFrameExplicit = true;
        }
        else
        {
          options->startFrame = parsed;
        }
      }
    }
    else if (arg[0] == '-')
    {
      fprintf(stderr, "Unknown option: %s\n", arg);
      return false;
    }
    else if (options->sampleName == NULL)
    {
      options->sampleName = arg;
    }
    else if (options->outputPath == NULL)
    {
      options->outputPath = arg;
    }
    else
    {
      fprintf(stderr, "Unexpected argument: %s\n", arg);
      return false;
    }
  }

  if (!options->maxFrameExplicit && !options->exactFrames.empty())
  {
    for (int frame : options->exactFrames)
    {
      if (frame > options->maxFrame)
      {
        options->maxFrame = frame;
      }
    }
  }

  return true;
}

int main(int argc, char* argv[])
{
  Options options;
  if (!parse_options(argc, argv, &options))
  {
    print_usage(stderr);
    return 1;
  }

  if (options.help)
  {
    print_usage(stdout);
    return 0;
  }

  if (options.listJson)
  {
    print_samples_json(stdout);
    return 0;
  }

  if (options.sampleName == NULL)
  {
    print_usage(stderr);
    print_samples_text(stderr);
    return 1;
  }

  int sampleIndex = find_sample_index(options.sampleName);
  if (sampleIndex < 0)
  {
    fprintf(stderr, "Sample not found: %s\n", options.sampleName);
    print_samples_text(stderr);
    return 1;
  }

  SampleContext context;
  context.hertz = 60.0f;
  context.subStepCount = 4;
  context.enableSleep = true;
  context.enableWarmStarting = true;
  context.enableContinuous = true;

  Sample* sample = g_sampleEntries[sampleIndex].CreateFcn(&context);
  if (!sample)
  {
    fprintf(stderr, "Failed to create sample\n");
    return 1;
  }

  fprintf(stderr, "Running sample: %s\n", options.sampleName);

  FILE* out = stdout;
  if (options.outputPath)
  {
    out = fopen(options.outputPath, "w");
    if (!out)
    {
      fprintf(stderr, "Failed to open output file: %s\n", options.outputPath);
      return 1;
    }
  }

  fprintf(out, "{\"checkpoints\":[");

  int checkpointCount = 0;
  for (int frame = 0; frame <= options.maxFrame; frame++)
  {
    if (frame > 0)
    {
      sample->Step();
    }

    if (should_dump_frame(options, frame))
    {
      dump_bodies(out, sample->m_worldId, frame, checkpointCount > 0);
      checkpointCount++;

      if (all_bodies_asleep(sample->m_worldId) && frame >= 100)
      {
        fprintf(stderr, "All bodies asleep at frame %d, terminating.\n", frame);
        break;
      }
    }
  }

  fprintf(out, "]}");
  if (out != stdout) fclose(out);

  delete sample;
  fprintf(stderr, "Done. %d checkpoints written.\n", checkpointCount);
  return 0;
}
