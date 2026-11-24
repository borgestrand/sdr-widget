#!/bin/bash
# Back up the sdr-widget tree to Google Drive as a time-stamped .tgz
# Usage, from Cygwin: sh ./backup.sh
# Excluded: git history, build output and .wav/.flac test files (tools/make_test_wavs.py makes them again)

SRC_PARENT=/cygdrive/e/kitten/avr32/github
DEST_DIR="/cygdrive/g/My Drive/sdr-widget-backup"
DEST="$DEST_DIR/sdr-widget-$(date +%Y%m%d-%H%M).tgz"

cd "$SRC_PARENT" || exit 1
mkdir -p "$DEST_DIR" || exit 1

tar cvzf "$DEST" \
	--exclude='sdr-widget/.git' \
	--exclude='*.o' \
	--exclude='*.d' \
	--exclude='*.wav' \
	--exclude='*.WAV' \
	--exclude='*.flac' \
	--exclude='*.FLAC' \
	--exclude='sdr-widget/Debug' \
	--exclude='sdr-widget/Release' \
	sdr-widget || exit 1

echo "Backup written to $DEST"
