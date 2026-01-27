#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

// Edge Impulse headers
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "model-parameters/model_metadata.h"
#include "model-parameters/model_variables.h"

/* 
    @function classify_frame
    @description Bu fonksiyon OpenCV Mat formatındaki bir görüntüsünü alıyor,
                    model başlangıçta RGB formatında eğitildiğinden görüntüyü RGB formatına çeviriyor,
                    ardından görüntüyü Edge Impulse SDK'nın header dosyalarında tanımlanan sinyal formatına dönüştürüyor
                    ve son olarak sınıflandırıcıyı çalıştırarak sonucu result parametresine yazıyor.
    @param frame: Giriş görüntüsü (cv::Mat formatında)
    @param result: Sınıflandırma sonucu (ei_impulse_result_t formatında)

*/
void classify_frame(const cv::Mat& frame, ei_impulse_result_t& result) {
    // Resimi model giriş boyutuna yeniden boyutlandır (96x96)
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT));
    
    // BGR'den RGB'ye dönüştürür
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    // Edge Impulse, pikselleri paketlenmiş uint32 RGB formatında (0x00RRGGBB) bekler
    // Toplam piksel sayısını hesapla
    size_t pixel_count = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT; 
    std::vector<float> features(pixel_count); // Her piksel için yer tut.
    
    // Pikselleri paketlenmiş uint32 formatına dönüştür, RGB format şu şekilde paketlenir
    size_t idx = 0;
    for (int y = 0; y < rgb.rows; y++) {
        for (int x = 0; x < rgb.cols; x++) {
            cv::Vec3b pixel = rgb.at<cv::Vec3b>(y, x);
            // Pikseli paketle (R = 255, G = 128, B = 64  (3 ayrı değer))
            uint32_t packed_pixel = (static_cast<uint32_t>(pixel[0]) << 16) |  // R
                                    (static_cast<uint32_t>(pixel[1]) << 8) |   // G
                                    (static_cast<uint32_t>(pixel[2]));         // B
            
            features[idx++] = static_cast<float>(packed_pixel);
        }
    }
    
    // Özelliklerden sinyal oluştur
    ei::signal_t signal;
    signal.total_length = pixel_count;  // Toplam piksel sayısı
    
    // get_data callback fonksiyonunu ayarla
    auto get_data_fn = [&features](size_t offset, size_t length, float *out_ptr) -> int {
        for (size_t i = 0; i < length; i++) {
            out_ptr[i] = features[offset + i];
        }
        return 0;
    };

    // sinyale get_data fonksiyonunu ata
    signal.get_data = get_data_fn;
    
    // Sınıflandırıcıyı çalıştır
    run_classifier(&ei_default_impulse, &signal, &result, false);
}

int main(int argc, char** argv) {
    
    // Meta bilgileri yazdırıyorum.
    std::cout << "Edge Impulse ile Gerçek zamanlı sınıflandırılmış nesne tespiti" << std::endl;
    std::cout << "Model: " << ei_default_impulse.impulse->project_name << std::endl;
    std::cout << "Version: " << ei_default_impulse.impulse->deploy_version << std::endl;
    std::cout << "Labels: ";
    
    
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        std::cout << ei_classifier_inferencing_categories[i];
        if (i < EI_CLASSIFIER_LABEL_COUNT - 1) std::cout << ", ";
    }
    std::cout << std::endl << std::endl;
    
    /*
        Raspberry Pi Pico kartıma ait kamera modülü olmadığından dolayı telefonumu IP kamera olarak kullandım ancak sizler dilerseniz
        doğrudan VideoCapture(0) ile bağlı olan kameranızı da kullanabilirsiniz.
    */

    // Uygulamanın IP kamerasına bağlanması
    std::string stream_url = "http://192.168.2.4:4747/video";
    std::cout << "Kameraya bağlanıyorum: " << stream_url << std::endl;
    
    // Video'yu aç
    cv::VideoCapture cap(stream_url);
    
    // Açılmadıysa console error ver ve çık
    if (!cap.isOpened()) {
        std::cerr << "\nHata: IP kamera akışı açılamadı!" << std::endl;
        std::cerr << "IP kamera uygulamasının şu adreste çalıştığından emin olun: " << stream_url << std::endl;
        return 1;
    }
    
    std::cout << "IP kamera başarıyla bağlandı!" << std::endl;
    std::cout << "Çıkmak için 'q' tuşuna basın" << std::endl << std::endl;
    
    cv::Mat frame;
    ei_impulse_result_t result = {0};
    
    // Video akış döngüsü
    while (true) {
        // Kare yakala
        cap >> frame;
        
        if (frame.empty()) {
            std::cerr << "Hata: Boş kare" << std::endl;
            break;
        }
        
        // Sınıflandırmayı çalıştır
        classify_frame(frame, result);
        
        // En yüksek tahmini buluyorum burada f denilmesinin sebebi float olmasıdır.
        float max_score = 0.0f;
        int max_idx = 0; // En yüksek skora sahip etiketin indeksi sadece en yüksek değer işe yaramıyor hangisine ait bu değer :)
        
        // Kaç adet label var ise ona kadar sayıyor ve en yüksek skoru ve indeksini buluyor
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            // Eğer bu skor şimdiye kadarki en yüksek skordan büyük ise güncelle
            if (result.classification[i].value > max_score) {
                max_score = result.classification[i].value;
                max_idx = i;
            }
        }
        
        // Ekrana çizimler yapılıyor buralar yapay zekaya ait.
        std::string label = ei_classifier_inferencing_categories[max_idx];
        std::string confidence = std::to_string(int(max_score * 100)) + "%";
        std::string text = label + " (" + confidence + ")";
        
        // Arka plan dikdörtgenini çiz
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, nullptr);
        cv::rectangle(frame, cv::Point(10, 10), 
                     cv::Point(20 + textSize.width, 50 + textSize.height), 
                     cv::Scalar(0, 0, 0), -1);
        
        // Metni çiz
        cv::putText(frame, text, cv::Point(15, 40), 
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        
        // Sol tarafta tüm tahminleri çiz
        int y_offset = 100;
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            std::string pred_text = ei_classifier_inferencing_categories[i];
            pred_text += ": " + std::to_string(int(result.classification[i].value * 100)) + "%";
            
            cv::Scalar color = (i == max_idx) ? cv::Scalar(0, 255, 0) : cv::Scalar(200, 200, 200);
            cv::putText(frame, pred_text, cv::Point(10, y_offset), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);
            y_offset += 30;
        }
        
        // Çıkarım süresini çiz
        float total_ms = (result.timing.dsp_us + result.timing.classification_us) / 1000.0f;
        std::string time_text = "Inference: " + std::to_string(int(total_ms)) + " ms";
        cv::putText(frame, time_text, cv::Point(10, y_offset + 20), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        // Frame'i göster
        cv::imshow("Edge Impulse Canlı Sınıflandırma", frame);
        
        // 'q' tuşuna basıldığında çık
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
    
    // Objelerin atanmış bellek tutucularını sil
    cap.release();
    cv::destroyAllWindows();
    
    std::cout << "\nKamera kapatıldı." << std::endl;
    
    return 0;
}
