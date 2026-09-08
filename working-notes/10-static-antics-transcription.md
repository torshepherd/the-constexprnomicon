# Static Antics transcription attempt — September 8, 2026

Tor requested another attempt at a full transcript of his [Static Antics talk](https://www.youtube.com/watch?v=6epTVJt1bpY), specifically using local yt-dlp and OpenAI Whisper on CPU.

**Outcome: no transcript and no audio downloaded.** Local Whisper works; fetching YouTube media remains the blocker. Nothing is running in the background.

## Prior attempt

[Context and decisions](01-context-and-decisions.md) records the earlier empty automatic-caption responses and transcript endpoint HTTP 400 `FAILED_PRECONDITION`. Those failures did not establish that audio downloading would also fail, so this session tested it separately.

## What worked

- Installed yt-dlp 2026.08.19, CPU-only PyTorch 2.14.0+cpu, and `openai-whisper`. FFmpeg was already available.
- yt-dlp fetched the watch page and player metadata. Title: `Static Antics by Tor Shepherd | Boston C++ Meetup March 2025`; channel: `C++ Boston Meetup`; duration: 5,126 seconds (1:25:26).
- The default extraction used the visionOS player API and discovered audio-only formats 139, 140, and 251, among other formats.
- Downloaded Whisper's `base.en` model (approximately 139 MiB), loaded it with `whisper.load_model('base.en', device='cpu')`, and verified the model was on CPU. This verifies installation/model loading, not transcription accuracy or throughput.
- The environment exposed eight CPUs' worth of quota and a 14 GiB memory limit.

## What failed

1. Default best-audio download chose format 251 (Opus). The media request timed out, including its retry. The HLS manifest lookup also timed out.
2. Loading the saved metadata and selecting format 140 (AAC) also timed out.
3. A small HTTP Range request for format 140 also timed out, without yielding an audio chunk.
4. Android player extraction supplied a combined video/audio format 18. Its request timed out on a different `googlevideo.com` media host.

No usable media file was created. The scratch metadata JSON is not a transcript, and its temporary signed media URLs are not durable artifacts.

After installing Whisper dependencies, yt-dlp encountered a certificate verification error. Using `--compat-options no-certifi` restored extraction with the environment's configured trust store; certificate verification was not disabled. Media timeouts persisted. The precise cause of the media-host timeouts was not established; do not describe them as a confirmed YouTube bot challenge or confirmed network-policy rejection.

## Resume

The practical next input is an uploaded audio/video recording, or another accessible source file. A future environment may also have different YouTube connectivity. Packages and model weights installed here are ephemeral and may need reinstalling.

Basic local setup and transcription commands, once audio is available:

```sh
python -m pip install torch --index-url https://download.pytorch.org/whl/cpu
python -m pip install openai-whisper yt-dlp
python -m whisper recording.m4a --model base.en --device cpu --fp16 False --language en --task transcribe --threads 8 --output_format all --output_dir transcript
```

The transcription command above was not run because no recording was available. Consider a larger English model for the final technical transcript; `base.en` was only an installation check. Preserve timestamps and mark uncertain C++ identifiers instead of silently inventing code from recognition errors. Full transcript coverage must include the complete recording, including any Q&A.
