# TinyML Projesi

Gömülü sistemler için makine öğrenimi uygulamaları.

## Kurulum

### Gereksinimler
- C++20 uyumlu derleyici (GCC 10+, Clang 10+, veya MSVC 2019+)
- CMake 3.15 veya üzeri
- Conan paket yöneticisi
- Arduino IDE (dağıtım için)

### Kurulum Adımları

1. Repoyu klonlayın:
```bash
git clone https://github.com/randomizeduser2/tinyml-on-microchips
cd TinyML
```

2. Conan'ı kurun (eğer yoksa):
```bash
pip install conan
```

3. Conan profilini ayarlayın:
```bash
conan profile detect --force
```

4. Bağımlılıkları yükleyin:
```bash
conan install . --output-folder=build --build=missing
```

5. Projeyi derleyin:
```bash
conan build .
```
### Model Mimarisi
- Giriş Katmanı: Sensör verileri (kamera, IP cam vb.)
- Çıkış Katmanı: 3 sınıf (Normal, Hata, Bilinmeyen)
- Peak Ram Usage: 232.9K
- Flash usage: 546.1K
- Kesinlik: 80.0%
- Kayıp: 0.54

### Cihaza Dağıtım
1. Modeli TensorFlow Lite formatına dönüştürün
2. Arduino IDE kullanarak mikrodenetleyicinize yükleyin
3. Seri port üzerinden sınıflandırmaları izleyin

## Lisans

MIT License


