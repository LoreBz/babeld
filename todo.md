# TO DO

- supporto per IPv6...

### NePa
- come sapere tutte le interfacce di un host mininet e lanciare comando bkgrnd su tutte?
  - pensavo: in background lancio uno script python invece che il comanda babel. In particolare:
  - lo script prima recupera i nomi delle interfacce
  - poi con os.system lancia davvero babeld con tando di interfacce appena recuperate
