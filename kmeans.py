import sys

EPSILON = 0.001
DEFAULT_ITER = 400


def error(msg="An Error Has Occurred"):
    print(msg)
    

# checks if the string represents a whole number, like 3, 3.0, or 3.
def is_int(s):
    try:
        x = float(s)
    except ValueError:
        return False

    return x.is_integer()


def distance(p, q):
    return sum((p[i] - q[i]) ** 2 for i in range(len(p)))


def closest(point, centroids):
    return min(range(len(centroids)), key=lambda i: distance(point, centroids[i]))

#calculates new centroid of one cluster
def mean(points, old_centroid):
    #points = vectors currently assigned to cluster
    #previous center of cluster
    if not points:
        #if the cluster is empty, keep the old centroid.
        return old_centroid[:]

    dim = len(old_centroid) #num of coords in each point
    return [sum(p[i] for p in points) / len(points) for i in range(dim)]


def kmeans(points, k, max_iter):
    centroids = [p[:] for p in points[:k]] #copy first k points as initial centroids

    for _ in range(max_iter):
        clusters = [[] for _ in range(k)]
        
        for p in points:
            cluster_id = closest(p, centroids)  #Find the closest cluster for p,
            clusters[cluster_id].append(p)      #then add p to that cluster’s list.

        new_centroids = [mean(clusters[i], centroids[i]) for i in range(k)]

        if all(distance(centroids[i], new_centroids[i]) < (EPSILON*EPSILON) for i in range(k)):
            centroids = new_centroids
            break

        centroids = new_centroids

    return centroids


def main():
    if len(sys.argv) not in [2, 3]:
        error()
        return

    if not is_int(sys.argv[1]):
        error("Incorrect number of clusters!")
        return

    k = int(float(sys.argv[1]))

    if len(sys.argv) == 3:
        if not is_int(sys.argv[2]):
            error("Incorrect maximum iteration!")
            return

        max_iter = int(float(sys.argv[2]))
    else:
        max_iter = DEFAULT_ITER

    if max_iter <= 1 or max_iter >= 800:
        error("Incorrect maximum iteration!")
        return

    points = [
        [float(x) for x in line.strip().split(",")]
        for line in sys.stdin
        if line.strip() != ""
    ]

    if k <= 1 or k >= len(points):
        error("Incorrect number of clusters!")
        return

    centroids = kmeans(points, k, max_iter)

    for c in centroids:
        print(",".join("%.4f" % x for x in c))


if __name__ == "__main__":
    main()