import pygame

# Inicializácia knižnice pygame
pygame.init()

# Cesta k .mp3 súboru, ktorý chcete prehrať
mp3_file = "song.mp3"

# Vytvorenie prehrávača
pygame.mixer.init()
pygame.mixer.music.load(mp3_file)

# Prehranie .mp3 súboru
pygame.mixer.music.play()

# Pockáme, kým sa pieseň prehrá
while pygame.mixer.music.get_busy():
    pass
