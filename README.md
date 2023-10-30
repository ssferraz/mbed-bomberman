![](./resources/official_armmbed_bomberman_badge.png)
# Mbed Bomberman

[![GitHub Actions](https://github.com/ssferraz/mbed-bomberman/workflows/CI/badge.svg)](https://github.com/ssferraz/mbed-bomberman/actions)

## Application functionality

O jogo "Mbed Bomberman" é uma versão simplificada do clássico jogo Bomberman, com as seguintes regras:

- Dois jogadores competem no jogo.
- Cada jogador pode soltar apenas uma bomba por vez.
- O tabuleiro é fixo e deve ter um tamanho mínimo de 76x72.
- Os personagens dos jogadores têm um tamanho de 2x2.
- As bombas têm o mesmo tamanho de 2x2.
- Quando uma bomba explode, ela cria uma explosão que tem tambanho máximo da linhaxcoluna.

## Building and running

1. Connect a USB cable between the USB port on the board and the host computer.
1. Run the following command to build the Bomberman project and program the microcontroller flash memory:

    ```bash
    $ git clone https://github.com/ssferraz/mbed-bomberman.git
    $ mbed deploy
    ```
    
Alternatively, you can manually copy the binary to the board, which you mount on the host computer over USB.

### License and contributions

The software is provided under Apache-2.0 license. Contributions to this project are accepted under the same license. Please see [CONTRIBUTING.md](./CONTRIBUTING.md) for more info.

This project contains code from other projects. The original license text is included in those source files. They must comply with our license guide.
