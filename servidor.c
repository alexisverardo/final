#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
 
#define BUF_SIZE 1024
 
void* sock_handler(void* param);


struct dato{
    char nick[BUF_SIZE];
};
struct lista{
    struct dato datos;
    struct lista *sig;
};
struct parametros {
    struct lista *l;
    int conn_sock;
};


struct lista *creanodo(void);
struct lista *insertacomienzo(struct lista *l, struct dato x);
void imprimelista(struct lista *l);

int main(int argc, char** argv) {
 
    char *error;
 
    int sockid,count;
    struct sockaddr_in direccion;
    char buffer[BUF_SIZE];
    int pid;
    pthread_t thread_id;

    struct parametros param;
 
 
    //creamos el socket inet-stream
    if((sockid=socket(PF_INET,SOCK_STREAM,0))<0){
        error="socket";
        goto err;
    }
 
 
    //seteamos la direccion en la que va a escuchar
    direccion.sin_family=AF_INET; //address family
    direccion.sin_port=htons(atoi(*(argv+1)));  //atoi ascii to integer
    direccion.sin_addr.s_addr=htonl(INADDR_ANY); //0.0.0.0
 
 
    //asociamos el socket con la direccion (bind)
    if((bind(sockid, (struct sockaddr *)&direccion, sizeof(direccion)))<0){
        error="bind";
        goto err;
    }
 
 
    // seteamos la cantidad de conexiones concurrentes en cola
    listen(sockid,1);
 
 
    //dejamos escuchando al proceso en el socket ip:puerto
    while(param.conn_sock=accept(sockid,NULL,NULL)){
        if (param.conn_sock<0){
            error="accept";
            goto err;
        }
 
        if(pthread_create(&thread_id,NULL,sock_handler, (void*)&param)){
            error="pthread_create";
            goto err;
        }
 
        printf("Conexion delegada al hilo %li\n",thread_id);
    }
 
    imprimelista(param.l);
 
 
    // ya tenemos un conn_sock de datos asociado con el cliente conectado
    return 0;
 
err:
    fprintf(stderr,"%d %s %s\n",errno,error,strerror(errno));
    exit(1);
 
}
void* sock_handler(void* param){
    struct parametros *p;
    p = (struct parametros *)param;
    struct dato x;
    char buffer[BUF_SIZE];
    char comparador;
    int conn_sock_th, count;
    pthread_detach(pthread_self());

 
    conn_sock_th = &p->conn_sock;
 
    printf("Iniciando hilo %li\n",pthread_self()); 

 
    while(count=recv(conn_sock_th,buffer,BUF_SIZE,0)){
        *(buffer+count)='\0';
        comparador = *buffer;
        if(comparador==';'){
            p->l=creanodo();
            *buffer=' ';
            strcpy(x.nick,buffer);
            p->l=insertacomienzo(&p->l,x);
            printf("Tu nick es : %s \n");
        }
        else{
        printf("Recibiendo datos (%li): %s\n",pthread_self(),buffer);
        }
    }
    close(conn_sock_th);
    pthread_exit(NULL);
 
}

struct lista *creanodo(void) {
  return (struct lista *) malloc(sizeof(struct lista));
}
struct lista *insertacomienzo(struct lista *l, struct dato x) {
  struct lista *p,*q;
  q = creanodo(); /* crea un nuevo nodo */
  q->datos = x; /* copiar los datos */
  q->sig = l;
  l = q;
  return l;
}
void imprimelista(struct lista *l) {
  struct lista *p;
  p = l;
  while (p != NULL) {
    printf("%s\n",p->datos.nick);
    p = p->sig;
  }
}