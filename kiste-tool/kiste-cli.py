#!/usr/bin/python3
import click
import shutil
import glob
import os

@click.group()
def cli():
    pass

@cli.command()
@click.argument('sdcard', type=click.Path(exists=True))
@click.argument('taste', type=click.IntRange(0, 9))
@click.argument('file', type=click.Path(exists=True))
def add(sdcard: str, taste: int, file: str):
    # neuen Filename bestimmen
    destPath = sdcard + os.path.sep + str(taste).zfill(2)
    if not os.path.exists(destPath):
        os.mkdir(destPath)
    filesFuerTaste = glob.glob(destPath + os.path.sep +  '[0-9][0-9][0-9].mp3')
    print(filesFuerTaste)

    if len(filesFuerTaste) == 0:
        nextFileNumber = 0
    else:
        maxFilename = os.path.basename(max(filesFuerTaste))
        maxFilename = maxFilename.replace('.mp3', '').lstrip('0')
        nextFileNumber = 1 if len(maxFilename) == 0 else int(maxFilename) + 1
    nextFilename = str(nextFileNumber).zfill(3) + '.mp3'

    # datei kopieren
    shutil.copy(file, destPath + os.path.sep + nextFilename)

    # playlist.txt schreiben
    playlistFile = open(destPath + os.path.sep + 'playlist.txt', 'a')
    playlistFile.write(str(nextFileNumber).zfill(3) + ' - ' + file + "\n")
    playlistFile.close()

    click.echo('Füge \'' + file + '\' zu Taste ' + str(taste) + ' hinzu. Track ' + nextFilename)

if __name__ == "__main__":
    cli()
