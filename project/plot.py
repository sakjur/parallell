from matplotlib import pyplot as plt
from matplotlib import animation
import json

data = []
t = 0;
with open('output') as in_f:
  for line in in_f:
    line = line.strip()
    line = line.split(' ')
    line[0] = int(line[0])
    line[2] = float(line[2])
    line[3] = float(line[3])
    if line[0] > t:
      t = line[0]
    data.append(line)

figure = plt.figure()
plt.xkcd()
plt.title('N-body Simulation')
frames = []
for i in range(0, t, 5):
  frame = filter(lambda datum: datum[0] == i, data)
  xs = list(map(lambda f: f[2], frame))
  ys = list(map(lambda f: f[3], frame))
  frames.append((plt.scatter(xs, ys),))
  print("added frame %d" % i)

gif = animation.ArtistAnimation(figure, frames, interval=50,
    repeat_delay=3000, blit=True)

gif.save('nbody.mp4', fps=24, extra_args=['-vcodec', 'libx264'])
print("Printed animation to 'nbody.mp4'")

plt.show()

