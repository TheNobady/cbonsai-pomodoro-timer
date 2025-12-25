# cbonsai-pomodoro-timer
`cbonsai` is a bonsai tree generator, written in `C` using `ncurses`. I have taken inspirations from `cbonsai` and made a small pomodoro timer that uses the `cbonsai`'s algorithm 

## Manul
Ensure you have the ncurses development libraries installed:
`sudo dnf install ncurses-devel`

After that

```bash
git clone https://github.com/TheNobady/cbonsai-pomodoro-timer.git
cd cbonsai-pomodoro-timer
make install
```

## Usage
Run `./cbonsai-timer -w 25 -b 5`

## Options
- `-w [minutes]` : Set work duration (default 25)
- `-b [minutes]` : Set break duration (default 5)

## Credits
This project was inspired by and utilizes algorithms from [cbonsai](https://gitlab.com/jallbrit/cbonsai/) by [jallbrit](https://gitlab.com/jallbrit) 
