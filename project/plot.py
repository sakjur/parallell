from matplotlib import pyplot as plt
from matplotlib import animation
import json

data = None
with open('output.json') as json_in:
  data = json.loads(json_in.read())

figure = plt.figure()
plt.xkcd()
plt.title('N-body Simulation')
frames = []
for i in range(0, len(data), 5):
  frame = data[str(i)]
  xs = list(map(lambda body: frame[body]['x'], frame))
  ys = list(map(lambda body: frame[body]['y'], frame))
  frames.append((plt.scatter(xs, ys),))

gif = animation.ArtistAnimation(figure, frames, interval=50,
    repeat_delay=3000, blit=True)

gif.save('nbody.mp4', fps=24, extra_args=['-vcodec', 'libx264'])

#plt.show()

