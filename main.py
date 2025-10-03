import aiohttp
import asyncio

async def post_request():
    url = 'http://10.251.2.126/core'
    headers = {'Content-Type': 'application/json'}
    body = {'command': 'register_card'}
    
    async with aiohttp.ClientSession() as session:
        try:
            # Melakukan POST request
            async with session.post(url, json=body, headers=headers) as response:
                # Tunggu sekitar 3 detik untuk mendapatkan respon
                
                if response.status == 200:
                    # Jika status OK, ambil UID dari JSON respon
                    data = await response.json()
                    uid = data.get('uid', 'UID tidak ditemukan')
                    print(f"UID: {uid}")
                elif response.status == 400:
                    # Jika status 400, ambil message dari respon JSON
                    data = await response.json()
                    message = data.get('message', 'Pesan tidak ditemukan')
                    print(f"Message: {message}")
                else:
                    print(f"Terjadi kesalahan: {response.status}")
        except Exception as e:
            print(f"Terjadi error: {e}")

# Jalankan event loop untuk async request
async def main():
    await post_request()

if __name__ == '__main__':
    asyncio.run(main())
