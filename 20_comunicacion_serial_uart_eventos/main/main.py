import tkinter as tk
import serial

def send_command(command):
    if ser and ser.is_open:
        ser.write(command.encode())
        print(f"Enviado: {command}")

def close_app():
    if ser and ser.is_open:
        ser.close()
    root.destroy()

# Conectar al puerto COM8
try:
    ser = serial.Serial("COM3", 9600)
    print("Conectado a COM3")
except serial.SerialException:
    print("No se pudo conectar a COM3")
    ser = None

# Crear ventana principal
root = tk.Tk()
root.title("Control Serial")
#root.geometry("400x400")

# Botones
btn_red = tk.Button(root, text="Rojo (R)", bg="red", fg="white", font=("Arial", 14), command=lambda: send_command('LedRojo'))
btn_green = tk.Button(root, text="Verde (G)", bg="green", fg="white", font=("Arial", 14), command=lambda: send_command('LedVerde'))
btn_blue = tk.Button(root, text="Azul (B)", bg="blue", fg="white", font=("Arial", 14), command=lambda: send_command('LedAzul'))
btn_off = tk.Button(root, text="Apagar (O)", bg="gray", fg="white", font=("Arial", 14), command=lambda: send_command('OFF'))

btn_red.pack(pady=10)
btn_green.pack(pady=10)
btn_blue.pack(pady=10)
btn_off.pack(pady=10)

# Cerrar aplicación de forma segura
root.protocol("WM_DELETE_WINDOW", close_app)

# Ejecutar la aplicación
tk.mainloop()
