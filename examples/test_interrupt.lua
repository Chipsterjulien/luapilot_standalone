-- test_interrupt2.lua
print("PID:", luapilot.pid())
print("Dans 5 secondes, tu auras 30 secondes pour faire kill -USR1 " .. luapilot.pid())

luapilot.signal.handle("USR1", function()
    print(">>> callback SIGUSR1 exécuté")
    -- pas de os.exit ici : on laisse le sleep retourner
end)

luapilot.sleep(5, 's')

print("c'est parti, kill -USR1 " .. luapilot.pid())
local t0 = luapilot.monotonic()
local ok_, err = luapilot.sleep(30, 's')
local elapsed = luapilot.monotonic() - t0

print(string.format("sleep retour : ok=%s, err=%s, elapsed=%.3fs",
    tostring(ok_), tostring(err), elapsed))
print("Script continue normalement après l'interruption")
