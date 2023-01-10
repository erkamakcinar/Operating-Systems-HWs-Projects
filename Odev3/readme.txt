Ödev macOS ortamında hazırlanmıştır. Derleyici olarak gcc kullanılmıştır.

NOT: 
"$" sembolü terminal komutu olduğunu belirtmek için kullanılmıştır. Komut "$" sembolünden sonra gelen kod parçacığıdır

Derlemek için gerekli terminal komutu:
$gcc -o a.out odev3.c

Çalıştırmak için gerekli terminal komutu:
$./a.out 121643 5000 1020 a.txt

macOS ortamında çalıştırılabilir bir dosyanın 3 segmenti vardır. Daha detaylı bilgi vermesi açısından size -m komutunun çıktısını size.png dosyasında bulabilirsiniz.
Bunlar sırasıyla SEGMENT__TEXT, SEGMENT__DATA, SEGMENT__LINK'dir.
Bu yüzden programım parametre olarak 3 adet integer segmentSize bilgisi alacaktır.
Bu üç parametre sırasıyla yukarıdaki sırayla aynıdır. Yani yukarıdaki verilen komutla çalıştırırsanız; 
SEGMENT__TEXT'in değeri 121643 olur.
4. parametre reference string dosyasının ismidir. a.txt dosyası çalıştırılma ortamında bulunmalı ve
her satırı boşluk ile ayrılmış 2 adet integer değer barındırmalı.
Örnek bir dosya ekleyeceğim fakat burada da göstermek adına dosyayın içeriği örneğin şu olabilir:
0 0
0 1
0 2
0 3
0 4
0 5
0 6
0 7
0 8
0 9
0 10
0 1
0 11
0 12
0 13
0 14
0 15
Ödevi hazırlarken varsayımlarım:
1) Eğer erişilecek page memory'de yoksa, memory'ye getirdikten sonra tlb'ye de eklediğini varsaydım.
2) Herhangi bir page'i memory'den çıkarılınca tlb table'dan da çıkarılacağını varsaydım.
3) Page'i tlb içinde bulursa lru stack'ı güncelledim.
Örneğin: page 3 için önce tlb'de olup olmadığına bakar, tlb'de bulursa en son kullanılan page olarak da stack'i günceller.
Gerekli delay'i alır. Bulamazsa memory'ye bakar. Memory içinde olanlar stack'lerin içerikleridir.
Memory'de bulursa stack'i günceller, page'i tlb table'a ekler. Gerekli delay'i alır.
Memory'de bulamazsa memory'ye getirir ve page'i tlb table'a ekler. Gerekli delay'i alır.

Musa Erkam Akçınar
191104083