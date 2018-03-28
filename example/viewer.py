import pygame
from PyDemux import Video

import time


def main():
    width, height = -1, -1
    screen = None

    pygame.init()

    video = Video.open('/Users/tee/Downloads/interview.mp4')

    while 1:
        im = video.get_frame()
        if im is None:
            break

        if im.size != (width, height):
            width, height = im.size

            pygame.display.set_mode((width, height))
            screen = pygame.display.get_surface()

        image = pygame.image.frombuffer(im.tobytes(), im.size, "RGB")
        screen.blit(image, (0, 0))
        pygame.display.flip()

        pygame.event.get()
        time.sleep(1)


main()