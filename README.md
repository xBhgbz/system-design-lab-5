# Лабораторная работа №5: Оптимизация производительности через кеширование и rate limiting

### Выполнил Кудрин Ярослав Владимирович, М8О-102СВ-25, вариант 13


## Кеширование 

Для кеширования я использовал Redis.

Подробнее о стратегии кеширования в [perfomance_design.md](performance_design.md)


## Rate Limiting

Реализован алгоритм **Token Bucket** для защиты от DDoS и спама.

Подробнее о Rate Limiting в [perfomance_design.md](performance_design.md)

## Запуск

```bash
docker-compose up --build
```

Сервис будет доступен на `http://localhost:8080`


## Тесты

```bash
bash tests.sh
```

