for f in ./*.mesh; do
  assimp export $f "${f%.*}.obj"
    # do some stuff here with "$f"
    # remember to quote it or spaces may misbehave
done